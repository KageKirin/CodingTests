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


static unsigned char data[2048];	//queue heap

struct Q
{
	unsigned char queue[80];
	unsigned char queue_first;
	unsigned char queue_last;
	
	void* operator new(size_t size, void* memory);
	void operator delete(void* memory);
	Q();
	~Q();
};

void* Q::operator new(size_t size, void* memory)
{
	return memory;
}

void Q::operator delete(void* memory)
{
}

Q::Q():
queue_first(0),
queue_last(0)
{
	memset(queue, 0, 80);
}

Q::~Q()
{
}


Q* create_queue()
{
	return new (data) Q;
}


void destroy_queue(Q* q)
{
	delete q;
}


void enqueue_byte(Q* q, unsigned char b)
{
	q->queue[q->queue_last] = b;
	++q->queue_last;

	if(q->queue_last >= 80)
	{
		on_out_of_memory();
	}
}


unsigned char dequeue_byte(Q* q)
{
	if(q->queue_first > q->queue_last)
	{
		on_illegal_operation();	
	}
	
	unsigned char b = q->queue[q->queue_first];
	++q->queue_first;
	return b;
}


// error handling
void on_out_of_memory()
{
	printf("out of memory\n");
	while(true)
	{}
}

void on_illegal_operation()
{
	printf("illegal operation\n");
	while(true)
	{}
}



