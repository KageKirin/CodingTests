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
    enqueue_byte(qtest, 0);
    enqueue_byte(qtest, 1);
    enqueue_byte(qtest, 2);
    enqueue_byte(qtest, 3);
    EXPECT_EQ(dequeue_byte(qtest), 0);
    EXPECT_EQ(dequeue_byte(qtest), 1);
    EXPECT_EQ(dequeue_byte(qtest), 2);
    EXPECT_EQ(dequeue_byte(qtest), 3);
}


int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

