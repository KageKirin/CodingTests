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
static byteType current_queue_max_length = 80;
static Q*const queue_ids = (Q*)&data[0];	//we use the memory area [0..63], first 64 bytes


//array alias for queued bytes
static const unsigned int remaining_space = data_size - max_queue_info_byteSize;
static const unsigned int max_queued_byte_count = remaining_space / sizeof(queued_byte);
static queued_byte*const queued_bytes = (queued_byte*)&data[max_queue_count];	//we use the remaining data for storing the data




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
		qb->value = BAD_VALUE;
	}
}

void Q::enqueue_byte(byteType b)
{
	//	bound_check();
	
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
	static unsigned int tempBufferSize = 20;
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

//


static void Q_bonds_check()
{
}


static void Q_size_adjust()
{
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
	
	Q_bonds_check();
	Q_size_adjust();	//check if q count > 24, if so adjust queue max size to less
	
	q->start_offset = Q::get_start_offset(q);
	q->length = 0;
	
	return q;
}


void destroy_queue(Q* q)
{
	//	printf("destroying Q [0x%p] with id: %i \n", q, *q);
	Q::destroy(q);
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


