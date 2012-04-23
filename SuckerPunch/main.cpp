//
//  main.cpp
//  SuckerPunch
//
//  Created by Christian Helmich on 22.04.12.
//  Copyright (c) 2012 Christian Helmich. All rights reserved.
//

#include "stdafx.h"
#include <cstdio>
#include "Queue.h"

int main(int argc, const char * argv[])
{
	printf("Hello, Suckerpunch!\n");
	
	//test
	Q* qtest = create_queue();
	
	enqueue_byte(qtest, 0);
	enqueue_byte(qtest, 1);
	enqueue_byte(qtest, 2);
	enqueue_byte(qtest, 3);

	printf("%d\n", dequeue_byte(qtest));
	printf("%d\n", dequeue_byte(qtest));
	printf("%d\n", dequeue_byte(qtest));
	printf("%d\n", dequeue_byte(qtest));
	
	destroy_queue(qtest);
	
	printf("Test output!\n");

	//final output
	Q* q0 = create_queue();
	enqueue_byte(q0, 0);
	enqueue_byte(q0, 1);
	Q* q1 = create_queue();
	enqueue_byte(q1, 3);
	enqueue_byte(q0, 2);
	enqueue_byte(q1, 4);
	printf("%d ", dequeue_byte(q0));
	printf("%d\n", dequeue_byte(q0));
	enqueue_byte(q0, 5);
	enqueue_byte(q1, 6);
	printf("%d ", dequeue_byte(q0));
	printf("%d\n", dequeue_byte(q0));
	destroy_queue(q0);
	printf("%d ", dequeue_byte(q1));
	printf("%d ", dequeue_byte(q1));
	printf("%d\n", dequeue_byte(q1));
	destroy_queue(q1);
	
	//Stress testing
	printf("Creation/desctruction stress test!\n");
	Q* stq[65];
	for(int i = 0; i < 64; ++i)
	{
		stq[i] = create_queue();
	}
	//stq[64] = create_queue();	//must fail
	for(int i = 0; i < 64; ++i)
	{
		destroy_queue(stq[i]);
	}
	//destroy_queue(stq[0]);	//must fail
	//destroy_queue(stq[64]);	//must fail
	
	for(int i = 0; i < 64; ++i)
	{
		stq[i+0] = create_queue();
		stq[i+1] = create_queue();
		stq[i+2] = create_queue();
	
		destroy_queue(stq[i+2]);
		destroy_queue(stq[i+1]);
		destroy_queue(stq[i+0]);	//will run out of queues at 62 if this line is commented out
	}
	
	printf("Queue filling stress test!\n");
	Q* fillQ[15];
	for(int i = 0; i < 15; ++i)
	{
		fillQ[i] = create_queue();
	}
	
	for(int a = 0; a < 8; ++a)
	{
		for(int i = 0; i < 15; ++i)
		{
			for(int b = 0; b < 10; ++b)
			{
				enqueue_byte(fillQ[i], (unsigned char)((a*i + b)%256));
			}
		}
	}
	
	for(int i = 0; i < 15; ++i)
	{
		for(int c = 0; c < 80; ++c)
		{
			printf("q(%i): %i", i, dequeue_byte(fillQ[i]));
		}
		printf("\n");
		destroy_queue(fillQ[i]);
	}
	
    return 0;
}

