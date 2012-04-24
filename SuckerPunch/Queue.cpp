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

//queue structure
template<unsigned int _size, unsigned int _qcount, typename T>
class queue_structure
{
private:
	typedef T value_type;
	struct queued_value
	{
		Q queueID;
		value_type value;
	};
	
	static const unsigned int data_size = _size;
	byteType data[data_size];	//queue heap

	static const unsigned int max_queue_count = _qcount;
	Q* queue_ids;	//we use the memory area [0..63], first 64 bytes

	static const unsigned int max_queued_value_count = (data_size - max_queue_count) / sizeof(queued_value);
	queued_value* queued_values;	//we use the remaining data for storing the data
	
	
private:	
	Q *const getStartQ()
	{
		return &queue_ids[0];
	}
	
	Q *const getEndQ()
	{
		return &queue_ids[max_queue_count];
	}
	
	queued_value *const getStartQueuedValue()
	{
		return &queued_values[0];
	}
	
	queued_value *const getEndQueuedValue()
	{
		return &queued_values[max_queued_value_count];
	}
	
	//conditions
	bool queueInValidRange(Q* q)
	{
		Q* qstart = getStartQ();
		return int(q - qstart) < max_queue_count; 
	}
	
	bool queuedValueInValidRange(queued_value* qb)
	{
		queued_value* qbstart = getStartQueuedValue();	
		return int(qb - qbstart) < max_queued_value_count; 
	}
	
	
	bool queuesAreInitialized()
	{
		for(Q* q = getStartQ();
			q != getEndQ();
			++q)
		{
			if(*q != BAD_VALUE)
				return true;
		}
		return false;
	}
		
	byteType getNextFreeQID()
	{
		static byteType count = 0;
		byteType rv = count;
		++count;
		bool changed = true;
		while(changed)
		{
			for(Q* q = getStartQ(); queueInValidRange(q); ++q)
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
		for(Q* firstUninitQ = getStartQ();
			firstUninitQ != getEndQ();
			++firstUninitQ)
		{
			if(*firstUninitQ == BAD_VALUE)
				return firstUninitQ;
		}
		assert_OutOfMemory(false);	//out of queues
		return NULL;
	}
	
	
	void destroyQueuedValues(Q* q)
	{
		for(queued_value* qb = getStartQueuedValue();
			qb != getEndQueuedValue();
			++qb)
		{
			if(qb->queueID == *q)
				memset(qb, BAD_VALUE, sizeof(queued_value));
		}
	}
	
	queued_value* getFirstAvailableQueuedValue()
	{
		queued_value* firstUninitQB = getStartQueuedValue();
		while(firstUninitQB->queueID != BAD_VALUE)
		{
			assert_OutOfMemory(queuedValueInValidRange(firstUninitQB));	//out of memory
			++firstUninitQB;
		}
		return firstUninitQB;	
	}


	queued_value* getNextHole(queued_value* qbstart)
	{
		queued_value* qb = qbstart;
		while(qb->queueID != BAD_VALUE)
		{
			if(!queuedValueInValidRange(qb))
				return NULL;
			++qb;
		}
		return qb;
	}

	queued_value* getNextValid(queued_value* qbstart)
	{
		queued_value* qb = qbstart;
		while(qb->queueID == BAD_VALUE)
		{
			if(!queuedValueInValidRange(qb))
				return NULL;
			++qb;
		}
		if(!queuedValueInValidRange(qb))
			return NULL;
		return qb;
	}

	void swapQueuedValues(queued_value* a, queued_value* b)
	{
		assert_IllegalOp(a != NULL);
		assert_IllegalOp(b != NULL);
		
		assert_IllegalOp(queuedValueInValidRange(a));
		assert_IllegalOp(queuedValueInValidRange(b));

		memcpy(a, b, sizeof(queued_value));
		memset(b, BAD_VALUE, sizeof(queued_value));
	}

	void fillUpHolesInQueuedValues()
	{
		queued_value* qbHoleSearchStart = getStartQueuedValue();
		queued_value* qbFirstHole = getNextHole(qbHoleSearchStart);
		while(qbFirstHole != NULL)	//NULL -> no holes, nothing to do
		{
		//		if(qbFirstHole != qbHoleSearchStart)
		
			queued_value* qbNextValid = getNextValid(qbFirstHole + 1);
			if(qbNextValid == NULL)
				break; //no more valids, nothing to swap
		
			swapQueuedValues(qbFirstHole, qbNextValid);
			qbHoleSearchStart = qbFirstHole +1;
			qbFirstHole = getNextHole(qbHoleSearchStart);
		}
	}
	
	queued_value* getFirstQueuedValueForQ(Q* q)
	{
		assert_IllegalOp(queuesAreInitialized());
		assert_IllegalOp(q != NULL);
		assert_IllegalOp(queueInValidRange(q));

		queued_value* firstQB = getStartQueuedValue();
		while(firstQB->queueID != *q)
		{
			assert_IllegalOp(queuedValueInValidRange(firstQB));
			++firstQB;
		}
		return firstQB;	
	}

public:
	queue_structure():
		queue_ids( &data[0] ),
		queued_values( (queued_value*)&data[max_queue_count] )
	{
		memset(data, BAD_VALUE, data_size);
	}
	
	Q* createQueue()
	{	
		Q* q = getFirstAvailableQ();
		*q = getNextFreeQID();
		
		//	printf("created new Q [0x%p] with id: %i \n", q, *q);
		return q;
	}
	
	void destroyQueue(Q* q)
	{
		//	printf("destroying Q [0x%p] with id: %i \n", q, *q);
		assert_IllegalOp(q != NULL);
		assert_IllegalOp(*q != BAD_VALUE);
		assert_IllegalOp(queueInValidRange(q));
		destroyQueuedValues(q);
		*q = BAD_VALUE;
	}
	
	void enqueueValue(Q* q, value_type b)
	{
		assert_IllegalOp(queuesAreInitialized());
		assert_IllegalOp(q != NULL);
		assert_IllegalOp(queueInValidRange(q));

		//here we need eventually to memmov_left the queued_values to avoid holes
		fillUpHolesInQueuedValues();
		
		//once this is done, we can get the first available qb
		queued_value* qb = getFirstAvailableQueuedValue();
		
		qb->queueID = *q;
		qb->value = b;
	}
	
	unsigned char dequeueValue(Q* q)
	{
		queued_value* qb = getFirstQueuedValueForQ(q);
		byteType b = qb->value;
		
		memset(qb, BAD_VALUE, sizeof(queued_value));
		
		return b;
	}

};

//simple singleton	//- code will need to be changed for more than 1 data field with same datatype
template <unsigned int _size, unsigned int _qcount, typename T>
static queue_structure<_size, _qcount, T>& getQueueStruct()
{
	static queue_structure<_size, _qcount, T> queue_struct;
	return queue_struct;
}

#define QS	getQueueStruct<2048, 64, byteType>()



//interface function definition

Q* create_queue()
{		
	return QS.createQueue();
}

void destroy_queue(Q* q)
{
	QS.destroyQueue(q);
}

void enqueue_byte(Q* q, unsigned char b)
{
	QS.enqueueValue(q, b);
}

unsigned char dequeue_byte(Q* q)
{
	return QS.dequeueValue(q);
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



