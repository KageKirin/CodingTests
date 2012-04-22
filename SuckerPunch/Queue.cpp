//
//  Queue.cpp
//  SuckerPunch
//
//  Created by Christian Helmich on 22.04.12.
//  Copyright (c) 2012 Christian Helmich. All rights reserved.
//

#include <cstdio>

static unsigned char data[2048];	//queue heap

Q* create_queue()
{
}


void destroy_queue(Q* q)
{
}


void enqueue_byte(Q* q, unsigned char b)
{
}


unsigned char dequeue_byte(Q* q)
{
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
