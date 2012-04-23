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

static const byteType BAD_VALUE = 0xFF;

//assert
static void assertIllegalOp(bool cond)
{
	if(!cond)
		on_illegal_operation();
}

static void assertOutOfMemory(bool cond)
{
	if(!cond)
		on_out_of_memory();
}


template<unsigned int _size, typename T>
struct queue_structure
{
	typedef T value_type;
	static const unsigned int data_size = 2048;
	byteType data[data_size];	//queue heap

	static const unsigned int max_queue_count = 64;
	Q* queue_ids;	//we use the memory area [0..63], first 64 bytes

	struct queued_byte
	{
		Q queueID;
		value_type value;
	};
	
	static const unsigned int remaining_space = data_size - max_queue_count;
	static const unsigned int max_queued_byte_count = remaining_space / sizeof(queued_byte);
	queued_byte* queued_bytes;	//we use the remaining data for storing the data
	
	queue_structure():
	queue_ids( &data[0] ),
	queued_bytes( (queued_byte*)&data[max_queue_count] )
	{
		memset(data, BAD_VALUE, data_size);
	}
	
	
	//conditions
	bool queueInValidRange(Q* q)
	{
		Q* qstart = &queue_ids[0];
	
		//	printf("q[0x%p] - qstart[0x%p] = %i < %i ? %i \n",
		//	q, qstart,
		//	int(q - qstart),
		//	max_queue_count,
		//	(int(q - qstart) < max_queue_count));
	
		return int(q - qstart) < max_queue_count; 
	}
	
	
	bool queuesAreInitialized()
	{
	for(Q* q = &queue_ids[0];
		queueInValidRange(q);
		++q)
		{
		if(*q != BAD_VALUE)
			return true;
		}
	return false;
	}
	
	
	bool queuedByteInValidRange(queued_byte* qb)
	{
	queued_byte* qbstart = &queued_bytes[0];	
	return int(qb - qbstart) < max_queued_byte_count; 
	}
	
	
	
	byteType getNextFreeQID()
	{
	static byteType count = 0;
	byteType rv = count;
	++count;
	bool changed = true;
	while(changed)
		{
		for(Q* q = &queue_ids[0]; queueInValidRange(q); ++q)
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
	
	Q* getFirstAvailableQ()
	{	
		for(Q* firstUninitQ = &queue_ids[0];
			queueInValidRange(firstUninitQ);
			++firstUninitQ)
			{
			if(*firstUninitQ == BAD_VALUE)
				return firstUninitQ;
			}
		assertOutOfMemory(false);	//out of queues
		return NULL;
	}

};


template <unsigned int _size, typename T>
static queue_structure<_size, T>* getQueueStruct()
{
	static queue_structure<_size, T> queue_struct;
	return queue_struct;
}


Q* create_queue()
{	
	Q* q = getFirstAvailableQ();

	*q = getNextFreeQID();
	
	//	printf("created new Q [0x%p] with id: %i \n", q, *q);
	return q;
}


static void destroyQueuedBytes(Q* q)
{
	for(queued_byte* qb = &queued_bytes[0];
		queuedByteInValidRange(qb);
		++qb)
	{
		if(qb->queueID == *q)
			memset(qb, BAD_VALUE, sizeof(queued_byte));
	}
}

void destroy_queue(Q* q)
{
	//	printf("destroying Q [0x%p] with id: %i \n", q, *q);
	assertIllegalOp(q != NULL);
	assertIllegalOp(*q != BAD_VALUE);
	assertIllegalOp(queueInValidRange(q));
	destroyQueuedBytes(q);
	*q = BAD_VALUE;
}


static queued_byte* getFirstAvailableQueuedByte()
{
	queued_byte* firstUninitQB = &queued_bytes[0];
	while(firstUninitQB->queueID != BAD_VALUE)
	{
		assertOutOfMemory(queuedByteInValidRange(firstUninitQB));	//out of memory
		++firstUninitQB;
	}
	return firstUninitQB;	
}


static queued_byte* getNextHole(queued_byte* qbstart)
{
	queued_byte* qb = qbstart;
	while(qb->queueID != BAD_VALUE)
	{
		if(!queuedByteInValidRange(qb))
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
		if(!queuedByteInValidRange(qb))
			return NULL;
		++qb;
	}
	if(!queuedByteInValidRange(qb))
		return NULL;
	return qb;
}

static void swapQueuedBytes(queued_byte* a, queued_byte* b)
{
	assertIllegalOp(a != NULL);
	assertIllegalOp(b != NULL);
	
	assertIllegalOp(queuedByteInValidRange(a));
	assertIllegalOp(queuedByteInValidRange(b));

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
	assertIllegalOp(queuesAreInitialized());
	assertIllegalOp(q != NULL);
	assertIllegalOp(queueInValidRange(q));

	//here we need eventually to memmov_left the queued_bytes to avoid holes
	fillUpHolesInQueuedBytes();
	
	//once this is done, we can get the first available qb
	queued_byte* qb = getFirstAvailableQueuedByte();
	
	qb->queueID = *q;
	qb->value = b;
}


static queued_byte* getFirstQueuedByteForQ(Q* q)
{
	assertIllegalOp(queuesAreInitialized());
	assertIllegalOp(q != NULL);
	assertIllegalOp(queueInValidRange(q));

	queued_byte* firstQB = &queued_bytes[0];
	while(firstQB->queueID != *q)
	{
		assertIllegalOp(queuedByteInValidRange(firstQB));
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



