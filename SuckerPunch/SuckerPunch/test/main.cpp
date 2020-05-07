/*
* main.cpp
* SuckerPunch
* Unitests for SuckerPunch Queue
* Created by Francisco Pedraza on 06.05.20.
*/

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
#define STRESS_QUEUE 65
#define STRESS_FAIL 62

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
	/*
	Tests random small values for queue
	*/
    enqueue_byte(qtest, 0);
    enqueue_byte(qtest, 1);
    enqueue_byte(qtest, 2);
    enqueue_byte(qtest, 3);
    EXPECT_EQ(dequeue_byte(qtest), 0);
    EXPECT_EQ(dequeue_byte(qtest), 1);
    EXPECT_EQ(dequeue_byte(qtest), 2);
    EXPECT_EQ(dequeue_byte(qtest), 3);
}

// Positive Big
TEST_F(SuckerPunchTest, QueueBigger) {
	/*
	Tests some random bigger values for queue
	*/
    enqueue_byte(qtest, 10);
    enqueue_byte(qtest, 20);
    enqueue_byte(qtest, 40);
    enqueue_byte(qtest, 60);
    EXPECT_EQ(dequeue_byte(qtest), 10);
    EXPECT_EQ(dequeue_byte(qtest), 20);
    EXPECT_EQ(dequeue_byte(qtest), 40);
    EXPECT_EQ(dequeue_byte(qtest), 60);
}

// Negative Queue
TEST_F(SuckerPunchTest, QueueNegative) {
	/*
	Tests that the negative values are not queued
	*/
    enqueue_byte(qtest, -1);
    enqueue_byte(qtest, -2);
    enqueue_byte(qtest, -3);
    enqueue_byte(qtest, -1000);
    EXPECT_NE(dequeue_byte(qtest), -1);
    EXPECT_NE(dequeue_byte(qtest), -2);
    EXPECT_NE(dequeue_byte(qtest), -3);
    EXPECT_NE(dequeue_byte(qtest), -1000);
}

// Negative Queue
TEST_F(SuckerPunchTest, Zero) {
    enqueue_byte(qtest, 0);
    EXPECT_EQ(dequeue_byte(qtest), 0);
}

// Stres Testing
TEST_F(SuckerPunchTest, OutOfMemory) {
	Q* ooMQ[FILL_STRESS];
	for(i = 0; i < FILL_STRESS; i++)
	{
		ooMQ[i] = create_queue();
		EXPECT_TRUE(ooMQ[i]);
	}

	for(a = 0; a < 8; a++)
	{
		for(i = 0; i < FILL_STRESS; i++)
		{
			for(b = 0; b < 10; b++)
			{
				enqueue_byte(ooMQ[i], (unsigned char)((a*i + b) % 255));
			}
		}
	}
	enqueue_byte(ooMQ[0], 0xAA);

	for(i = 0; i < FILL_STRESS; i++)
	{

		for(c = 0; c < FULL_BYTES; c++)
		{
            //printf("q(%d): %i;\t");
            EXPECT_TRUE(dequeue_byte(ooMQ[i]));
		}
		printf("\n");
		destroy_queue(ooMQ[i]);
	}
}

// Illegal operation
TEST_F(SuckerPunchTest, IllegalOperation) {
	/*
	The purpose of this test is to verify that we don't
	trigger Illegal operation
	*/
	Q* iOp[ILLEGAL_QUEUES];
	int bytes_per_q[ILLEGAL_QUEUES] = {0};
	int start_memory = 1792;

	srand(time(NULL));

	int stress_query = (rand() % 63) + 1;

	for(i = 0; i < stress_query; ++i)
	{
		iOp[i] = create_queue();
		EXPECT_TRUE(iOp[i]);
		bytes_per_q[i] = rand() % (start_memory/stress_query);
		start_memory -= bytes_per_q[i];

		for(b = 0; b < bytes_per_q[i]; b++)
		{
			enqueue_byte(iOp[i], (unsigned char)((i*32 + b)%255));
		}
	}

	for(i = 0; i < stress_query; i++)
	{
		for(c = 0; c < bytes_per_q[i]; c++)
		{
            //printf("(%d)\t", i);
            EXPECT_TRUE(dequeue_byte(iOp[i]));
		}
		printf("\n");
		destroy_queue(iOp[i]);
	}
}

TEST_F(SuckerPunchTest, StressPass) {
	/*
	The purpose of this test is to verify that we are able to
	stres the memory without breaking it
	*/
	Q* stress_q[STRESS_QUEUE];
	for(i = 0; i < STRESS_FAIL; i++) // 65 mem full - out of memory
	{
		stress_q[i] = create_queue();
	}

	stress_q[64] = create_queue();

	for(i = 0; i < 62; i++) // 62 illegal operation
	{
		destroy_queue(stress_q[i]);
	}

	for(int i = 0; i < 50; i++) // had 50
	{
		stress_q[i+0] = create_queue();
		stress_q[i+1] = create_queue();
		stress_q[i+2] = create_queue();

		destroy_queue(stress_q[i+2]);
		destroy_queue(stress_q[i+1]);
		destroy_queue(stress_q[i+0]);
	}
}

TEST_F(SuckerPunchTest, StressFail) {
	/*
	The purpose of this test is to verify that we are able to
	get the memory limit (negative test case), so in this case
	it covers the memory full.
	*/
	Q* stress_q[STRESS_QUEUE];

	for(i = 0; i < STRESS_FAIL; i++) // 65 mem full - out of memory
	{
		try
		{
			int exp = 20;
			stress_q[i] = create_queue();
			if( i == STRESS_FAIL-1 ) {
				throw exp;
				EXPECT_ANY_THROW(exp);
				}
		}
		catch (int exp) {
			std::cout << "Stress Exception caught" << std::endl;
			break;
		}
	}
}


int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

