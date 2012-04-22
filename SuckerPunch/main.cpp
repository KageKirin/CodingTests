//
//  main.cpp
//  SuckerPunch
//
//  Created by Christian Helmich on 22.04.12.
//  Copyright (c) 2012 Christian Helmich. All rights reserved.
//

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
	
	/*
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
	*/
	
    return 0;
}

