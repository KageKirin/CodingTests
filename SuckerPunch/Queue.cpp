//
//  Queue.cpp
//  SuckerPunch
//
//  Created by Christian Helmich on 22.04.12.
//  Copyright (c) 2012 Christian Helmich. All rights reserved.
//

#include <cstdio>
#include <cstring>
#include "Queue.h"


struct queued_byte
{
	Q queueID;
	byteType value;
};

static const byteType BAD_VALUE = 0xFF;

static const unsigned int data_size = 2048;
static byteType data[data_size];	//queue heap

static const unsigned int max_queue_count = 64;
static Q* queue_ids = &data[0];	//we use the memory area [0..63], first 64 bytes

static const unsigned int remaining_space = data_size - max_queue_count;
static const unsigned int max_queued_byte_count = remaining_space / sizeof(queued_byte);
static queued_byte* queued_bytes = (queued_byte*)&data[max_queue_count];	//we use the remaining data for storing the data

static void initialize_data()
{
	static bool once = false;
	if(once)
		return;
	
	memset(data, BAD_VALUE, data_size);
	once = true;
}


//assert
static void assert_illegal_op(bool cond)
{
	if(!cond)
		on_illegal_operation();
}

static void assert_out_of_memory(bool cond)
{
	if(!cond)
		on_out_of_memory();
}


//conditions
static bool queue_is_valid(Q* q)
{
	Q* qstart = &queue_ids[0];
	
	printf("q[0x%p] - qstart[0x%p] = %i < %i ? %i \n",
		q, qstart,
		int(q - qstart),
		max_queue_count,
	   (int(q - qstart) < max_queue_count));
	
	return int(q - qstart) < max_queue_count; 
}


static bool queues_are_initialized()
{
	for(Q* q = &queue_ids[0];
		queue_is_valid(q);
		++q)
	{
		if(*q != BAD_VALUE)
			return true;
	}
	return false;
}


static bool queued_byte_is_valid(queued_byte* qb)
{
	queued_byte* qbstart = &queued_bytes[0];	
	return int(qb - qbstart) < max_queued_byte_count; 
}



static byteType getNextFreeQID()
{
	static byteType count = 0;
	byteType rv = count;
	++count;
	bool changed = true;
	while(changed)
	{
		for(Q* q = &queue_ids[0]; queue_is_valid(q); ++q)
		{
			if(*q == rv)
			{
				rv = count;
				++count;
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

static Q* getFirstAvailableQ()
{	
	for(Q* firstUninitQ = &queue_ids[0];
		queue_is_valid(firstUninitQ);
		++firstUninitQ)
	{
		if(*firstUninitQ == BAD_VALUE)
			return firstUninitQ;
	}
	assert_out_of_memory(false);	//out of queues
	return NULL;
}

Q* create_queue()
{
	initialize_data();
	Q* q = getFirstAvailableQ();

	*q = getNextFreeQID();
	
	printf("created new Q [0x%p] with id: %i \n", q, *q);
	return q;
}


static void destroyQueuedBytes(Q* q)
{
	for(queued_byte* qb = &queued_bytes[0];
		queued_byte_is_valid(qb);
		++qb)
	{
		if(qb->queueID == *q)
			memset(qb, BAD_VALUE, sizeof(queued_byte));
	}
}

void destroy_queue(Q* q)
{
	printf("destroying Q [0x%p] with id: %i \n", q, *q);
	assert_illegal_op(q != NULL);
	assert_illegal_op(*q != BAD_VALUE);
	assert_illegal_op(queue_is_valid(q));
	destroyQueuedBytes(q);
	*q = BAD_VALUE;
}


static queued_byte* getFirstAvailableQueuedByte()
{
	queued_byte* firstUninitQB = &queued_bytes[0];
	while(firstUninitQB->queueID != BAD_VALUE)
	{
		assert_out_of_memory(queued_byte_is_valid(firstUninitQB));	//out of memory
		++firstUninitQB;
	}
	return firstUninitQB;	
}


static queued_byte* getNextHole(queued_byte* qbstart)
{
	queued_byte* qb = qbstart;
	while(qb->queueID != BAD_VALUE)
	{
		if(!queued_byte_is_valid(qb))
			return NULL;
		++qb;
	}
	return qb;
}

static queued_byte* getNextValid(queued_byte* qbstart)
{
	queued_byte* qb = qbstart;
	while(qb->queueID == BAD_VALUE)
	{
		if(!queued_byte_is_valid(qb))
			return NULL;
		++qb;
	}
	if(!queued_byte_is_valid(qb))
		return NULL;
	return qb;
}

static void swapQueuedBytes(queued_byte* a, queued_byte* b)
{
	assert_illegal_op(a != NULL);
	assert_illegal_op(b != NULL);
	
	assert_illegal_op(queued_byte_is_valid(a));
	assert_illegal_op(queued_byte_is_valid(b));

	memcpy(a, b, sizeof(queued_byte));
	memset(b, BAD_VALUE, sizeof(queued_byte));
}

static void fillUpHolesInQueuedBytes()
{
	queued_byte* qbHoleSearchStart = &queued_bytes[0];
	queued_byte* qbFirstHole = getNextHole(qbHoleSearchStart);
	while(qbFirstHole != NULL)	//NULL -> no holes, nothing to do
	{
	//		if(qbFirstHole != qbHoleSearchStart)
	
		queued_byte* qbNextValid = getNextValid(qbFirstHole + 1);
		if(qbNextValid == NULL)
			break; //no more valids, nothing to swap
	
		swapQueuedBytes(qbFirstHole, qbNextValid);
		qbHoleSearchStart = qbFirstHole +1;
		qbFirstHole = getNextHole(qbHoleSearchStart);
	}
}

void enqueue_byte(Q* q, unsigned char b)
{
	assert_illegal_op(queues_are_initialized());
	assert_illegal_op(q != NULL);
	assert_illegal_op(queue_is_valid(q));

	//here we need eventually to memmov_left the queued_bytes to avoid holes
	fillUpHolesInQueuedBytes();
	
	//once this is done, we can get the first available qb
	queued_byte* qb = getFirstAvailableQueuedByte();
	
	qb->queueID = *q;
	qb->value = b;
}


static queued_byte* getFirstQueuedByteForQ(Q* q)
{
	assert_illegal_op(queues_are_initialized());
	assert_illegal_op(q != NULL);
	assert_illegal_op(queue_is_valid(q));

	queued_byte* firstQB = &queued_bytes[0];
	while(firstQB->queueID != *q)
	{
		assert_illegal_op(queued_byte_is_valid(firstQB));
		++firstQB;
	}
	return firstQB;	
}

unsigned char dequeue_byte(Q* q)
{
	queued_byte* qb = getFirstQueuedByteForQ(q);
	byteType b = qb->value;
	
	memset(qb, BAD_VALUE, sizeof(queued_byte));
	
	return b;
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



