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

//pair to keep a reference of the queue with the enqueued value
struct queued_byte
{
	Q queueID;
	byteType value;
};

//tag for uninitialized queues and data
static const byteType BAD_VALUE = 0xFF;


//overall available data
static const unsigned int data_size = 2048;
static byteType data[data_size];	//queue heap

//array alias for queues
static const unsigned int max_queue_count = 64;
static Q*const queue_ids = &data[0];	//we use the memory area [0..63], first 64 bytes
static Q*const Q_begin()
{
	return &queue_ids[0];
}

static Q*const Q_end()
{
	return &queue_ids[max_queue_count];
}

//array alias for queued bytes
static const unsigned int remaining_space = data_size - max_queue_count;
static const unsigned int max_queued_byte_count = remaining_space / sizeof(queued_byte);
static queued_byte*const queued_bytes = (queued_byte*)&data[max_queue_count];	//we use the remaining data for storing the data
static queued_byte*const queued_bytes_begin()
{
	return &queued_bytes[0];
}

static queued_byte*const queued_bytes_end()
{
	return &queued_bytes[max_queued_byte_count];
}


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



//data needs to be initialized once to BAD_DATA
static void initializeData()
{
	static bool once = false;
	if(once)
		return;
	
	memset(data, BAD_VALUE, data_size);
	once = true;
}


//conditions to check for data validity
static bool Q_is_in_valid_range(Q* q)
{
	Q* qstart = Q_begin();
	return int(q - qstart) < max_queue_count; 
}

static bool Q_at_least_one_exists()
{
	for(Q* q = Q_begin(); q != Q_end(); ++q)
	{
		if(*q != BAD_VALUE)
			return true;
	}
	return false;
}

static bool queued_byte_is_in_valid_range(queued_byte* qb)
{
	queued_byte* qbstart = queued_bytes_begin();	
	return int(qb - qbstart) < max_queued_byte_count; 
}


//helper functions
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

static Q* Q_get_first_available()
{	
	for(Q* q_1st_free = Q_begin(); q_1st_free != Q_end(); ++q_1st_free)
	{
		if(*q_1st_free == BAD_VALUE)
			return q_1st_free;
	}
	assert_OutOfMemory(false);	//out of queues
	return NULL;
}

Q* create_queue()
{
	initializeData();
	Q* q = Q_get_first_available();
	assert_IllegalOp(q != NULL);	
	assert_IllegalOp(Q_is_in_valid_range(q));
	*q = Q_get_next_free_id();
	assert_IllegalOp(*q != BAD_VALUE);
	return q;
}


static void destroy_queued_bytes(Q* q)
{
	for(queued_byte* qb = queued_bytes_begin(); qb != queued_bytes_end(); ++qb)
	{
		if(qb->queueID == *q)
			memset(qb, BAD_VALUE, sizeof(queued_byte));
	}
}

void destroy_queue(Q* q)
{
	//	printf("destroying Q [0x%p] with id: %i \n", q, *q);
	assert_IllegalOp(q != NULL);
	assert_IllegalOp(*q != BAD_VALUE);
	assert_IllegalOp(q != Q_end());
	destroy_queued_bytes(q);
	*q = BAD_VALUE;
}


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

static void queued_byte_swap(queued_byte* a, queued_byte* b)
{
	assert_IllegalOp(a != NULL);
	assert_IllegalOp(b != NULL);
	
	assert_IllegalOp(queued_byte_is_in_valid_range(a));
	assert_IllegalOp(queued_byte_is_in_valid_range(b));

	memcpy(a, b, sizeof(queued_byte));
	memset(b, BAD_VALUE, sizeof(queued_byte));
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

void enqueue_byte(Q* q, unsigned char b)
{
	assert_IllegalOp(Q_at_least_one_exists());
	assert_IllegalOp(q != NULL);
	assert_IllegalOp(q != Q_end());

	//here we need eventually to fill "holes", i.e. queued_bytes that have already been dequeued, thus would turn up as "free" and change the order of the queue if used
	queued_byte_fill_holes();
	
	//once this is done, we can get the first available qb
	queued_byte* qb = queued_byte_get_first_available();
	
	qb->queueID = *q;
	qb->value = b;
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

unsigned char dequeue_byte(Q* q)
{
	queued_byte* qb = queued_byte_get_first_for_Q(q);
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



