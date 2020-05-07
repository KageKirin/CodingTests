//
//  Queue.h
//  SuckerPunch
//
//  Created by Christian Helmich on 22.04.12.
//  Copyright (c) 2012 Christian Helmich. All rights reserved.
//

#ifndef SuckerPunch_Queue_h
#define SuckerPunch_Queue_h

class Q;

Q* create_queue();			//Creates a FIFO byte queue, returning a handle to it.
void destroy_queue(Q* q);	//Destroy an earlier created byte queue.
void enqueue_byte(Q* q, unsigned char b);	//Adds a new byte to a queue.
unsigned char dequeue_byte(Q* q);			//Pops the next byte off the FIFO queue.

void on_out_of_memory();
void on_illegal_operation();

#endif
