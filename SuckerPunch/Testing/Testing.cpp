#include "pch.h"
#include "CppUnitTest.h"
#include "../SuckerPunch/Queue.cpp"
#include <chrono> 

#define MAX_NUMBER_OF_QUEUES  63
#define MAX_NUMBER_OF_TESTING  20
#define MAX_QUEUE_SIZE  2048
#define MAX_UNSIGNED_CHAR_SIZE 255
#define AVERAGE_NUM_QUEUES 15

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std::chrono;

namespace Testing
{
	TEST_CLASS(UnitTesting)
	{
	public:
		TEST_METHOD_CLEANUP(cleanup)
		{
			if(m_currentQueues.size() > 0)
			{ 
				Assert::IsTrue(m_currentQueues.size() == 0);
				Assert::IsFalse(gErrorOccured);
			}
			gErrorOccured = false;
		}
		TEST_METHOD(CanCreateAndDeleteAQueue)
		{
			Q* q= create_queue();
			Assert::IsNotNull(q);
			Assert::IsFalse(gErrorOccured);

			destroy_queue(q);
			Assert::IsFalse(gErrorOccured);
		}

		TEST_METHOD(CanCreateMultipleQueues)
		{
			unsigned short counter = 0;
			while (counter < MAX_NUMBER_OF_TESTING)
			{
				Q* q = create_queue();
				Assert::IsNotNull(q);
				Assert::IsFalse(gErrorOccured);

				destroy_queue(q);
				Assert::IsFalse(gErrorOccured);
				counter++;
			}
		}

		// Disabled due to bug: unable to destory all created queues
		/*TEST_METHOD(CanCreateMaxNumberOfQueues)
		{
			unsigned short counter = 0;
			Q* testQueues[AVERAGE_NUM_QUEUES];

			while (counter < MAX_NUMBER_OF_QUEUES)
			{
				testQueues[counter] = create_queue();
				Assert::IsNotNull(testQueues[counter]);
				Assert::IsFalse(gErrorOccured);
				counter++;
			}

			counter = 0;
			while (counter < MAX_NUMBER_OF_QUEUES)
			{
				destroy_queue(testQueues[counter]);
				Assert::IsFalse(gErrorOccured);
				counter++;
			}
		}*/

		TEST_METHOD(CanEnqueueAndDequeue)
		{
			Q* q = create_queue();
			Assert::IsNotNull(q);
			Assert::IsFalse(gErrorOccured);

			const unsigned expectedChar = 12;
			enqueue_byte(q, expectedChar);
			Assert::IsFalse(gErrorOccured);

			const unsigned resultChar = dequeue_byte(q);
			Assert::IsFalse(gErrorOccured);

			Assert::AreEqual(resultChar, expectedChar);

			destroy_queue(q);
			Assert::IsFalse(gErrorOccured);
		}

		TEST_METHOD(CanEnqueueAndDequeueMultipleQueues)
		{
			Q* q = create_queue();
			Assert::IsNotNull(q);
			Assert::IsFalse(gErrorOccured);
			Q* q1 = create_queue();
			Assert::IsNotNull(q1);
			Assert::IsFalse(gErrorOccured);

			const unsigned expectedChar = 12;
			const unsigned expectedChar1 = 24;
			enqueue_byte(q, expectedChar);
			Assert::IsFalse(gErrorOccured);
			enqueue_byte(q1, expectedChar1);
			Assert::IsFalse(gErrorOccured);

			const unsigned resultChar = dequeue_byte(q);
			Assert::IsFalse(gErrorOccured);
			const unsigned resultChar1 = dequeue_byte(q1);
			Assert::IsFalse(gErrorOccured);

			Assert::AreEqual(resultChar, expectedChar);
			Assert::AreEqual(resultChar1, expectedChar1);

			destroy_queue(q);
			Assert::IsFalse(gErrorOccured);
			destroy_queue(q1);
			Assert::IsFalse(gErrorOccured);
		}

		TEST_METHOD(CanEnqueueAndDequeueWithLongData)
		{
			const unsigned char data_count = 10;
			const unsigned char data[data_count] = { 0, 2, 4, 8, 10, 12, 14, 16, 20, 22 };

			Q* q = create_queue();
			Assert::IsNotNull(q);
			Assert::IsFalse(gErrorOccured);

			for (const unsigned char input : data)
			{
				enqueue_byte(q, input);
				Assert::IsFalse(gErrorOccured);
			}

			unsigned char result[data_count];
			for (unsigned short counter = 0; counter < data_count; counter++)
			{
				const unsigned resultChar = dequeue_byte(q);
				Assert::IsFalse(gErrorOccured);
				result[counter] = resultChar;
			}

			for (unsigned short counter = 0; counter < data_count; counter++)
			{
				Assert::AreEqual(data[counter], result[counter]);
			}

			destroy_queue(q);
			Assert::IsFalse(gErrorOccured);
		}

		TEST_METHOD(CanEnqueueAndDequeueWithLongDataMultipleQueues)
		{
			const unsigned char data_count = 10;
			const unsigned char data[data_count] = { 0, 2, 4, 8, 10, 12, 14, 16, 20, 22 };
			const unsigned char data1[data_count] = { 1, 3, 5, 7, 9, 11, 13, 15, 17, 19 };

			Q* q = create_queue();
			Assert::IsNotNull(q);
			Assert::IsFalse(gErrorOccured);
			Q* q1 = create_queue();
			Assert::IsNotNull(q);
			Assert::IsFalse(gErrorOccured);


			for (const unsigned char input : data)
			{
				enqueue_byte(q, input);
				Assert::IsFalse(gErrorOccured);
			}

			for (const unsigned char input : data1)
			{
				enqueue_byte(q1, input);
				Assert::IsFalse(gErrorOccured);
			}

			unsigned char result[data_count];
			unsigned char result1[data_count];
			for (unsigned short counter = 0; counter < data_count; counter++)
			{
				const unsigned resultChar = dequeue_byte(q);
				Assert::IsFalse(gErrorOccured);
				result[counter] = resultChar;

				const unsigned resultChar1 = dequeue_byte(q1);
				Assert::IsFalse(gErrorOccured);
				result1[counter] = resultChar1;
			}

			for (unsigned short counter = 0; counter < data_count; counter++)
			{
				Assert::AreEqual(data[counter], result[counter]);
				Assert::AreEqual(data1[counter], result1[counter]);
			}

			destroy_queue(q);
			Assert::IsFalse(gErrorOccured);

			destroy_queue(q1);
			Assert::IsFalse(gErrorOccured);
		}

		// BUG: if the queue is filled up it's max size, dequeued and destroyed we run into an issue
		/*TEST_METHOD(CanStoreMaxSize)
		{
			unsigned char data[MAX_QUEUE_SIZE];
			for (unsigned short counter = 0; counter < MAX_QUEUE_SIZE; counter++)
			{
				data[counter] = std::rand() % MAX_UNSIGNED_CHAR_SIZE;
			}

			Q* q = create_queue();
			Assert::IsNotNull(q);
			Assert::IsFalse(gErrorOccured);

			for (const unsigned char input : data)
			{
				enqueue_byte(q, input);
				Assert::IsFalse(gErrorOccured);
			}

			unsigned char result[MAX_QUEUE_SIZE];
			for (unsigned short counter = 0; counter < MAX_QUEUE_SIZE; counter++)
			{
				const unsigned resultChar = dequeue_byte(q);
				Assert::IsFalse(gErrorOccured);
				result[counter] = resultChar;
			}

			for (unsigned short counter = 0; counter < MAX_QUEUE_SIZE; counter++)
			{
				Assert::AreEqual(data[counter], result[counter]);
			}

			destroy_queue(q);
			Assert::IsFalse(gErrorOccured);
		}*/

		// Bug: Exception Code: C0000005 : access violation if try to store more than max size
		/*TEST_METHOD(CannotStorePastMaxSize)
		{
			unsigned char data[MAX_QUEUE_SIZE + 1];
			for (unsigned short counter = 0; counter < MAX_QUEUE_SIZE + 1; counter++)
			{
				data[counter] = std::rand() % MAX_UNSIGNED_CHAR_SIZE;
			}

			Q* q = create_queue();
			Assert::IsNotNull(q);
			Assert::IsFalse(gErrorOccured);

			for (const unsigned char input : data)
			{
				enqueue_byte(q, input);
				Assert::IsFalse(gErrorOccured);
			}

			Assert::IsTrue(gErrorOccured);
		}*/

		// Bug: Exception Code: C0000005 :can not destory the same queue multiple times
		/*TEST_METHOD(CannotDestoryAQueueMultipleTimes)
		{
			Q* q = create_queue();
			Assert::IsNotNull(q);
			Assert::IsFalse(gErrorOccured);

			destroy_queue(q);
			Assert::IsFalse(gErrorOccured);

			destroy_queue(q);
			Assert::IsTrue(gErrorOccured);
		}*/
		TEST_METHOD(StressTest)
		{
			const unsigned char rounds = 20;
			const unsigned char fillPerRound = 110;

			Q* testQueues[AVERAGE_NUM_QUEUES];

			for (unsigned short counter = 0; counter < AVERAGE_NUM_QUEUES; counter++)
			{
				testQueues[counter] = create_queue();
				Assert::IsFalse(gErrorOccured);
			}

			for (unsigned short k = 0; k < rounds; k++)
			{
				for (unsigned short i = 0; i < AVERAGE_NUM_QUEUES; i++)
				{
					for (unsigned short j = 0; j < fillPerRound; j++)
					{
						enqueue_byte(testQueues[i], (unsigned char)(std::rand() % MAX_UNSIGNED_CHAR_SIZE));
						Assert::IsFalse(gErrorOccured);
					}
				}

				for (unsigned short i = 0; i < AVERAGE_NUM_QUEUES; i++)
				{
					for (unsigned short c = 0; c < fillPerRound; c++)
					{
						const unsigned char value = dequeue_byte(testQueues[i]);
						Assert::IsFalse(gErrorOccured);
					}
				}
			}

			for (unsigned short i = 0; i < AVERAGE_NUM_QUEUES; i++)
			{
				destroy_queue(testQueues[i]);
				Assert::IsFalse(gErrorOccured);
			}
		}
		TEST_METHOD(StressTestForcedRerrangement)
		{
			Q* testQueues[MAX_NUMBER_OF_QUEUES];
			int bytes_per_q[MAX_NUMBER_OF_QUEUES] = { 0 };
			int start_memory = 1500;

			int remainder = (rand() % MAX_NUMBER_OF_QUEUES - 1) + 1;

			for (int i = 0; i < remainder; i++)
			{
				testQueues[i] = create_queue();

				bytes_per_q[i] = rand() % (start_memory / remainder);	
				start_memory -= bytes_per_q[i];

				for (int b = 0; b < bytes_per_q[i]; ++b)
				{
					enqueue_byte(testQueues[i], (unsigned char)((i * 32 + b) % MAX_UNSIGNED_CHAR_SIZE));

				}
			}

			for (int i = 0; i < remainder; ++i)
			{
				for (int c = 0; c < bytes_per_q[i]; ++c)
				{
					const unsigned char value = dequeue_byte(testQueues[i]);
				}
				destroy_queue(testQueues[i]);
			}
		}

		TEST_METHOD(PerformanceTestingQueueEnqueueTime)
		{
			const unsigned short enqueue_threshold = 50;
			const unsigned char fillPerRound = 110;

			Q* testQueues[AVERAGE_NUM_QUEUES];


			for (unsigned short counter = 0; counter < AVERAGE_NUM_QUEUES; counter++)
			{
				testQueues[counter] = create_queue();
			}

			for (unsigned short i = 0; i < AVERAGE_NUM_QUEUES; i++)
			{
				for (unsigned short j = 0; j < fillPerRound; j++)
				{
					auto start = high_resolution_clock::now();
					enqueue_byte(testQueues[i], (unsigned char)(std::rand() % MAX_UNSIGNED_CHAR_SIZE));
					auto stop = high_resolution_clock::now();
					auto duration = duration_cast<microseconds>(stop - start);

					Assert::IsTrue(duration.count() < enqueue_threshold);
				}
			}

			for (unsigned short i = 0; i < AVERAGE_NUM_QUEUES; i++)
			{
				destroy_queue(testQueues[i]);
				Assert::IsFalse(gErrorOccured);
			}
		}

		TEST_METHOD(PerformanceTestingQueueDequeueTime)
		{
			const unsigned short dequeue_threshold = 50;
			const unsigned char fillPerRound = 110;

			Q* testQueues[AVERAGE_NUM_QUEUES];


			for (unsigned short counter = 0; counter < AVERAGE_NUM_QUEUES; counter++)
			{
				testQueues[counter] = create_queue();
			}

			for (unsigned short i = 0; i < AVERAGE_NUM_QUEUES; i++)
			{
				for (unsigned short j = 0; j < fillPerRound; j++)
				{
					enqueue_byte(testQueues[i], (unsigned char)(std::rand() % MAX_UNSIGNED_CHAR_SIZE));
				}
			}

			for (unsigned short i = 0; i < AVERAGE_NUM_QUEUES; i++)
			{
				for (unsigned short c = 0; c < fillPerRound; c++)
				{
					auto start = high_resolution_clock::now();
					const unsigned char value = dequeue_byte(testQueues[i]);
					auto stop = high_resolution_clock::now();
					auto duration = duration_cast<microseconds>(stop - start);

					Assert::IsTrue(duration.count() < dequeue_threshold);
				}
			}

			for (unsigned short i = 0; i < AVERAGE_NUM_QUEUES; i++)
			{
				destroy_queue(testQueues[i]);
				Assert::IsFalse(gErrorOccured);
			}
		}

		TEST_METHOD(PerformanceTestingDestroyQueueTime)
		{
			const unsigned short destroy_threshold = 50;

			Q* testQueues[AVERAGE_NUM_QUEUES];

			for (unsigned short counter = 0; counter < AVERAGE_NUM_QUEUES; counter++)
			{
				testQueues[counter] = create_queue();
			}

			for (unsigned short i = 0; i < AVERAGE_NUM_QUEUES; i++)
			{
				auto start = high_resolution_clock::now();
				destroy_queue(testQueues[i]);
				auto stop = high_resolution_clock::now();
				auto duration = duration_cast<microseconds>(stop - start);

				Assert::IsTrue(duration.count() < destroy_threshold);
			}
		}

		TEST_METHOD(PerformanceTestingQueueCreationTime)
		{
			const unsigned short creation_threshold = 10;

			Q* testQueues[AVERAGE_NUM_QUEUES];

			for (unsigned short counter = 0; counter < AVERAGE_NUM_QUEUES; counter++)
			{
				auto start = high_resolution_clock::now();
				testQueues[counter] = create_queue();
				auto stop = high_resolution_clock::now();
				auto duration = duration_cast<microseconds>(stop - start);

				Assert::IsTrue(duration.count() < creation_threshold);

				destroy_queue(testQueues[counter]);
				Assert::IsFalse(gErrorOccured);
			}
		}
	};
}
