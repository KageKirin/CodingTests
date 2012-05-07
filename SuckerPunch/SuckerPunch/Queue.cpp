//
//  Queue.cpp
//  SuckerPunch
//
//  Created by Christian Helmich on 22.04.12.
//  Copyright (c) 2012 Christian Helmich. All rights reserved.
//

#include "stdafx.h"
#include <cstdio>
#include <cstring>
#include "Queue.h"


typedef unsigned char	byteType;
typedef unsigned short	uShort;

struct queued_byte
{
	byteType value;
	static uShort current_count;
	
	static queued_byte*const begin();
	static queued_byte*const end();
	static queued_byte*const at(unsigned int idx);
	
	static bool in_valid_range(queued_byte* qb);
	static bool in_valid_range(unsigned int idx);
	static bool memory_available();
	
	void invalidate();
};
uShort queued_byte::current_count = 0;

class Q
{
public:
	uShort start_offset;	//can exceed 255
	uShort length;			//can exceed 255

public:
	//API replication
	static Q* create();
	static void destroy(Q* q);

	void enqueue_byte(byteType b);
	byteType dequeue_byte();
	
	//queued bytes iterating
	queued_byte*const queued_bytes_begin();
	queued_byte*const queued_bytes_end();
	queued_byte*const queued_byte_at(unsigned int idx);
	
	//Q iterating
	static Q*const begin();
	static Q*const end();	
	static uShort get_start_offset(Q* q);
	
	//conditions to check for data validity
	static bool at_least_one_exists();
	bool in_valid_range();
	bool is_valid();
	bool length_is_valid();
	bool offset_is_valid();
	
	uShort get_queued_bytes_data_size();
	
protected:
	void destroy_queued_bytes();
	void move_queued_bytes(queued_byte*const targetBuffer);
	void shift_left_queued_bytes();
	
	void bound_check_and_memory_rearrange();
};


//min/max. not using std for it
template<typename T>
T MIN(T a, T b)
{
	return a < b ? a : b;
}

template<typename T>
T MAX(T a, T b)
{
	return a > b ? a : b;
}

//tag for uninitialized queues and data
static const byteType	BAD_VALUE = 0xFF;
static const uShort		BAD_QUEUE = 0xFFFF;

//overall available data
static const unsigned int data_size = 2048;
static byteType data[data_size];	//queue heap
									//data needs to be initialized once to BAD_DATA
static void initializeData()
{
	static bool once = false;
	if(once)
		return;
	
	memset(data, BAD_VALUE, data_size);
	once = true;
}


//array alias for queues
static const unsigned int max_queue_count = 64;
static const unsigned int max_queue_info_byteSize = max_queue_count * sizeof(Q);
static uShort current_queue_count = 0;
static uShort current_queue_max_length = 80;
static Q*const queue_ids = (Q*)&data[0];	//we use the memory area [0..63], first 64 bytes


//array alias for queued bytes
static const unsigned int remaining_space = data_size - max_queue_info_byteSize;
static const unsigned int max_queued_byte_count = remaining_space / sizeof(queued_byte);
static queued_byte*const queued_bytes = (queued_byte*)&data[max_queue_info_byteSize];	//we use the remaining data for storing the data




//assert functions
static void assert_IllegalOp(bool cond)
{
	if(!cond)
		on_illegal_operation();
}

static void assert_OutOfMemory(bool cond)
{
	if(!cond)
		on_out_of_memory();
}




// Q::functions implementation
Q* Q::create()
{
	for(Q* q_1st_free = Q::begin(); q_1st_free != Q::end(); ++q_1st_free)
	{
		if(q_1st_free->start_offset == BAD_QUEUE)
			return q_1st_free;
	}
	assert_OutOfMemory(false);	//out of queues
	return NULL;
}

void Q::destroy(Q *q)
{
	assert_IllegalOp(q != NULL);
	assert_IllegalOp(q->in_valid_range());
	assert_IllegalOp(q->is_valid());
	
	q->destroy_queued_bytes();
	
	q->start_offset = BAD_QUEUE;
	q->length = BAD_QUEUE;
}

Q*const Q::begin()
{
	return &queue_ids[0];
}

Q*const Q::end()
{
	return &queue_ids[max_queue_count];
}


queued_byte*const Q::queued_bytes_begin()
{
	return &queued_bytes[start_offset];	
}

queued_byte*const Q::queued_bytes_end()
{
	return &queued_bytes[start_offset + length];	
}


bool Q::in_valid_range()
{
	Q* qstart = Q::begin();
	return int(this - qstart) < max_queue_info_byteSize; 
}

bool Q::at_least_one_exists()
{
	for(Q* q = Q::begin(); q != Q::end(); ++q)
	{
		if(q->start_offset != BAD_VALUE)
			return true;
	}
	return false;
}

uShort Q::get_start_offset(Q* q)
{
	unsigned int qIdx = (q - Q::begin());
	//qIdx /= sizeof(Q);
	return qIdx * current_queue_max_length;
}

void Q::destroy_queued_bytes()
{
	assert_IllegalOp(length_is_valid());
	
	for(queued_byte* qb = queued_bytes_begin();
		qb != queued_bytes_end();
		++qb)
	{
		qb->invalidate();
	}
}

void Q::enqueue_byte(byteType b)
{
	assert_OutOfMemory(queued_byte::memory_available());
	
	bound_check_and_memory_rearrange();
	
	queued_byte* qb = queued_bytes_begin() + length;
	qb->value = b;
	++length;
	++queued_byte::current_count;
}

byteType Q::dequeue_byte()
{
	queued_byte* qb = queued_bytes_begin();
	byteType b = qb->value;
	
	shift_left_queued_bytes();
	--length;
	--queued_byte::current_count;

	return b;
}

bool Q::is_valid()
{
	return length_is_valid() && offset_is_valid();
}

bool Q::length_is_valid()
{
	return length != BAD_QUEUE;
}

bool Q::offset_is_valid()
{
	return start_offset != BAD_QUEUE;
}

uShort Q::get_queued_bytes_data_size()
{
	return uShort(length * sizeof(queued_byte));
}


void Q::move_queued_bytes(queued_byte*const targetBuffer)
{
#define tempBufferSize 20u
	queued_byte buffer[tempBufferSize];
	
	for(int i = 0; i < length; i += tempBufferSize)
	{
		unsigned int copySize = MIN(tempBufferSize, (unsigned int)(length - i));
		queued_byte* origin = queued_bytes_begin() + i;
		
		memcpy(buffer, origin, copySize);
		memcpy(targetBuffer + i, buffer, copySize);
		memset(origin, BAD_VALUE, copySize);
	}
}


void Q::shift_left_queued_bytes()
{
	queued_byte* qb_target = queued_bytes_begin();
	for(queued_byte* qb = qb_target + 1;
		qb != queued_bytes_end();
		++qb)
	{
		memcpy(qb_target, qb, sizeof(queued_byte));
		
		qb->invalidate();
		++qb_target;	
	}
}




// magic happens in here

static uShort memory_used_or_reserved()
{
	uShort sum = 0;
	for(Q* q = Q::begin(); q != Q::end(); ++q)
	{
		if(q->is_valid())
			sum += MAX(q->length, current_queue_max_length);
	}
	return sum;
}

static uShort memory_used()
{
	uShort sum = 0;
	for(Q* q = Q::begin(); q != Q::end(); ++q)
	{
		if(q->is_valid())
			sum += q->length;
	}
	return sum;
}

void Q::bound_check_and_memory_rearrange()
{
	assert_OutOfMemory(queued_byte::current_count + 1 < max_queued_byte_count);
	assert_IllegalOp(queued_byte::current_count == memory_used());	//something went very wrong if this one triggers
	
	uShort cur_ur_memory_length = memory_used_or_reserved();
	uShort new_ur_memory_length = cur_ur_memory_length + 1;
	uShort cur_res_memory_length = current_queue_count * current_queue_max_length;
	uShort opt_res_memory_length = max_queued_byte_count / current_queue_count;
	

	bool new_length_below_current_queue_max_length =
		length + 1 < current_queue_max_length;
	
	bool new_ur_memory_below_max_queued_byte_count =
		new_ur_memory_length < max_queued_byte_count;
	

	if(	new_length_below_current_queue_max_length
	&&	new_ur_memory_below_max_queued_byte_count )
	{
		return;	//nothing to do
	}
	
	
	if( !new_ur_memory_below_max_queued_byte_count )
	{
		if(opt_res_memory_length < current_queue_max_length)
		{
			// decrease current_queue_max_length
			// move queues accordingly
			
			byteType tempCopy[4096];	//do all the work into a copy to be on the safe side
			memset(tempCopy, BAD_VALUE, 4096);
			
			//copy data to new arrangement (in tempBuffer to avoid overwriting)
			uShort cumulOffsets = 0;
			uShort newOffsets[64] = {0};
			uShort* current_Q_new_offset = &newOffsets[0];
			for(Q* q = Q::begin(); q != Q::end(); ++q)
			{
				if(q->is_valid())
				{
					*current_Q_new_offset = cumulOffsets;
					memcpy(&tempCopy[*current_Q_new_offset], q->queued_bytes_begin(), q->get_queued_bytes_data_size());

					++current_Q_new_offset;
					cumulOffsets += MAX(opt_res_memory_length, q->get_queued_bytes_data_size());
					assert_IllegalOp(cumulOffsets < max_queued_byte_count);
				}
			}
			
			//copy newly arranged data back
			current_Q_new_offset = &newOffsets[0];
			for(Q* q = Q::begin(); q != Q::end(); ++q)
			{
				if(q->is_valid())
				{
					q->start_offset = *current_Q_new_offset;
					memcpy(q->queued_bytes_begin(), &tempCopy[*current_Q_new_offset], 
					   MAX(opt_res_memory_length, q->get_queued_bytes_data_size()));
				}
			}
			current_queue_max_length = opt_res_memory_length;
			
			
		}
		else if(opt_res_memory_length > current_queue_max_length)
		{
			current_queue_max_length = opt_res_memory_length;		
		}
		else
		{
		}
		//	change current_queue_max_length (smaller or better fitting)
		//	shift LEFT all the queues to new current_queue_max_length
	}
	
	if( !new_length_below_current_queue_max_length )
	{
		byteType tempCopy[4096];	//do all the work into a copy to be on the safe side
		memset(tempCopy, BAD_VALUE, 4096);
		
		if(opt_res_memory_length > current_queue_max_length)
		{
			current_queue_max_length = opt_res_memory_length;		
		}
		
		//copy data to new arrangement (in tempBuffer to avoid overwriting)
		uShort cumulOffsets = 0;
		uShort newOffsets[64] = {0};
		uShort* current_Q_new_offset = &newOffsets[0];
		for(Q* q = Q::begin(); q != Q::end(); ++q)
		{
			if(q->is_valid())
			{
				*current_Q_new_offset = cumulOffsets;
				assert_IllegalOp(*current_Q_new_offset < max_queued_byte_count);
				memcpy(&tempCopy[*current_Q_new_offset], q->queued_bytes_begin(), q->get_queued_bytes_data_size());
				
				++current_Q_new_offset;
				cumulOffsets += MAX(current_queue_max_length, q->get_queued_bytes_data_size());
				assert_IllegalOp(cumulOffsets < max_queued_byte_count);
			}
		}
		
		//copy newly arranged data back
		current_Q_new_offset = &newOffsets[0];
		for(Q* q = Q::begin(); q != Q::end(); ++q)
		{
			if(q->is_valid())
			{
				q->start_offset = *current_Q_new_offset;
				memcpy(q->queued_bytes_begin(), &tempCopy[*current_Q_new_offset], 
					   MAX(uShort(current_queue_max_length * sizeof(queued_byte)), q->get_queued_bytes_data_size()));
			}
		}

		
		
	//	change current_queue_max_length (greater / more optimized)
	//	shift RIGHT the queues AFTER this one, beginning with the last (or else, that will overwrite data)
	}
	

	
}


static void queued_byte_swap(queued_byte* a, queued_byte* b)
{
	assert_IllegalOp(a != NULL);
	assert_IllegalOp(b != NULL);
	
	assert_IllegalOp(queued_byte::in_valid_range(a));
	assert_IllegalOp(queued_byte::in_valid_range(b));
	
	memcpy(a, b, sizeof(queued_byte));
	memset(b, BAD_VALUE, sizeof(queued_byte));
}






//queued_byte::functions implementation
queued_byte*const queued_byte::begin()
{
	return &queued_bytes[0];
}

queued_byte*const queued_byte::end()
{
	return &queued_bytes[max_queued_byte_count];
}

queued_byte*const queued_byte::at(unsigned int idx)
{
	assert_IllegalOp(queued_byte::in_valid_range(idx));
	if(queued_byte::in_valid_range(idx))
		return &queued_bytes[idx];
	return NULL;
}

bool queued_byte::in_valid_range(queued_byte* qb)
{
	queued_byte* qbstart = queued_byte::begin();	
	return int(qb - qbstart) < max_queued_byte_count; 
}

bool queued_byte::in_valid_range(unsigned int idx)
{
	return idx < max_queued_byte_count; 
}

bool queued_byte::memory_available()
{
	return current_count < max_queued_byte_count;
}

void queued_byte::invalidate()
{
	memset(this, BAD_VALUE, sizeof(queued_byte));
}



// "API" implementation

Q* create_queue()
{
	initializeData();
	Q* q = Q::create();
	assert_IllegalOp(q != NULL);	
	assert_IllegalOp(q->in_valid_range());
	
	++current_queue_count;
	
	q->start_offset = Q::get_start_offset(q);
	q->length = 0;
	
	return q;
}


void destroy_queue(Q* q)
{
	//	printf("destroying Q [0x%p] with id: %i \n", q, *q);
	Q::destroy(q);
	
	--current_queue_count;
}


void enqueue_byte(Q* q, unsigned char b)
{
	assert_IllegalOp(q != NULL);
	assert_IllegalOp(q->is_valid());
	
	q->enqueue_byte(b);
}


unsigned char dequeue_byte(Q* q)
{
	assert_IllegalOp(q != NULL);
	assert_IllegalOp(q->is_valid());

	return q->dequeue_byte();
}


// error handling
void on_out_of_memory()
{
	printf("out of memory\n");
	while(true)
	{;}
}

void on_illegal_operation()
{
	printf("illegal operation\n");
	while(true)
	{;}
}



/// scrape





//helper functions
/*
 static byteType Q_get_next_free_id()
 {
 //avoid any possible ID conflict when creating and destroying lots of(>254) queues
 static byteType count = 0;
 byteType rv = count;
 count = (count + 1) % 255;	//0xFF is invalid
 bool changed = true;
 while(changed)
 {
 for(Q* q = Q_begin(); q != Q_end(); ++q)
 {
 if(*q == rv)
 {
 rv = count;
 count = (count + 1) % 255;
 changed = true;
 }
 else
 {
 changed = false;
 }	
 }
 }
 return rv;
 }
 */

/*
 static queued_byte* queued_byte_get_next_free(queued_byte* qbstart)
 {
 for(queued_byte* qb = qbstart; qb != queued_bytes_end(); ++qb)
 {		
 if(qb->queueID == BAD_VALUE)
 return qb;		
 }
 return NULL;	
 }
 
 static queued_byte* queued_byte_get_next_valid(queued_byte* qbstart)
 {
 for(queued_byte* qb = qbstart; qb != queued_bytes_end(); ++qb)	
 {
 if(qb->queueID != BAD_VALUE)
 return qb;
 }
 return NULL;
 }
 
 static queued_byte* queued_byte_get_first_available()
 {
 queued_byte* qb_1st_free = queued_byte_get_next_free(queued_bytes_begin());
 assert_OutOfMemory(qb_1st_free != NULL);	//out of memory
 return qb_1st_free;
 }
 
 static void queued_byte_fill_holes()
 {
 queued_byte* qbHoleSearchStart = queued_bytes_begin();
 queued_byte* qbFirstHole = queued_byte_get_next_free(qbHoleSearchStart);
 while(qbFirstHole != NULL)	//NULL -> no holes, nothing to do
 {
 //		if(qbFirstHole != qbHoleSearchStart)
 
 queued_byte* qbNextValid = queued_byte_get_next_valid(qbFirstHole + 1);
 if(qbNextValid == NULL)
 break; //no more valids, nothing to swap
 
 queued_byte_swap(qbFirstHole, qbNextValid);
 qbHoleSearchStart = qbFirstHole +1;
 qbFirstHole = queued_byte_get_next_free(qbHoleSearchStart);
 }
 }
 
 static queued_byte* queued_byte_get_first_for_Q(Q* q)
 {
 assert_IllegalOp(Q_at_least_one_exists());
 assert_IllegalOp(q != NULL);
 assert_IllegalOp(q != Q_end());
 
 queued_byte* qb;
 for(qb = queued_bytes_begin(); qb != queued_bytes_end(); ++qb)
 {
 if(qb->queueID == *q)
 return qb;		
 }
 assert_IllegalOp(queued_byte_is_in_valid_range(qb));
 return NULL;	
 }
 */


