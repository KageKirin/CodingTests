//
//  main.cpp
//  SuckerPunch
//
//  Created by Francisco Pedraza on 06.05.20.

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <gtest/gtest.h>
#include "../Queue.cpp"
#include "../stdafx.h"

/* Define constants for tests */
#define FILL_STRESS 15
#define FULL_BYTES 80
#define ILLEGAL_QUEUES 64

/* Needed vars */
int i, a, b, c;

class SuckerPunchTest : public ::testing::Test {
    protected:
    Q* qtest;

  void SetUp() override {
      qtest = create_queue();
  }

  void TearDown() override {
      destroy_queue(qtest);
  }
};

// Positive Test Scenario
TEST_F(SuckerPunchTest, QueueDequeue) {
    printf("Normal Queue and Dequeue!\n");
    enqueue_byte(qtest, 0);
    enqueue_byte(qtest, 1);
    enqueue_byte(qtest, 2);
    enqueue_byte(qtest, 3);
    EXPECT_EQ(dequeue_byte(qtest), 0);
    EXPECT_EQ(dequeue_byte(qtest), 1);
    EXPECT_EQ(dequeue_byte(qtest), 2);
    EXPECT_EQ(dequeue_byte(qtest), 3);
}

// Stres Testing
TEST_F(SuckerPunchTest, OutOfMemory) {
    printf("Out of memory test!\n");
	Q* fillQ[FILL_STRESS];
	for(i = 0; i < FILL_STRESS; i++)
	{
		fillQ[i] = create_queue();
	}

	for(a = 0; a < 8; a++)
	{
		for(i = 0; i < FILL_STRESS; i++)
		{
			for(b = 0; b < 10; b++)
			{
				enqueue_byte(fillQ[i], (unsigned char)((a*i + b) % 255));
			}
		}
	}
	enqueue_byte(fillQ[0], 0xAA);


	for(i = 0; i < FILL_STRESS; i++)
	{

		for(c = 0; c < FULL_BYTES; c++)
		{
            printf("q(%i): %i;\t");
            EXPECT_TRUE(dequeue_byte(fillQ[i]));
		}
		printf("\n");
		destroy_queue(fillQ[i]);
	}
}

// Illegal operation
TEST_F(SuckerPunchTest, IllegalOperation) {
    printf("Illegal Operation test!\n");
	Q* rearrQ[ILLEGAL_QUEUES];
	int bytes_per_q[ILLEGAL_QUEUES] = {0};
	int start_memory = 1792;

	srand(time(NULL));

	int qs_to_work_on = (rand() % 63) + 1;

	for(i = 0; i < qs_to_work_on; ++i)
	{
		rearrQ[i] = create_queue();
		bytes_per_q[i] = rand() % (start_memory/qs_to_work_on);
		start_memory -= bytes_per_q[i];

		for(b = 0; b < bytes_per_q[i]; b++)
		{
			enqueue_byte(rearrQ[i], (unsigned char)((i*32 + b)%255));
		}
	}

	for(i = 0; i < qs_to_work_on; i++)
	{
		for(c = 0; c < bytes_per_q[i]; c++)
		{
            printf("(%d)\t", i);
            EXPECT_TRUE(dequeue_byte(rearrQ[i]));
		}
		printf("\n");
		destroy_queue(rearrQ[i]);
	}
}

TEST_F(SuckerPunchTest, Stress) {
    printf("Creation/destruction stress test!\n");
	Q* stq[65];
	for(i = 0; i < 50; i++) // 65 mem full
	{
        printf("paco ");
        printf("%d :", i);
		stq[i] = create_queue();
	}

	stq[64] = create_queue();	//must fail
	for(i = 0; i < 50; i++) // 64 illegal operation
	{
        printf("paco2");
        printf("%d :", i);
		destroy_queue(stq[i]);
        //EXPECT_TRUE(destroy_queue(stq[i]));
	}
	//destroy_queue(stq[0]);	//must fail
	//destroy_queue(stq[64]);	//must fail

	for(int i = 0; i < 50; i++)
	{
        printf("paco3");
        printf("%d :", i);
		stq[i+0] = create_queue();
		stq[i+1] = create_queue();
		stq[i+2] = create_queue();

		destroy_queue(stq[i+2]);
		destroy_queue(stq[i+1]);
		destroy_queue(stq[i+0]);	//will run out of queues at 62 if this line is commented out
	}
}


int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

