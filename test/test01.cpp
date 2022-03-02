#include <gtest/gtest.h>
#include <iostream>
#include <cstdint>
#include <thread>
#include <stdexcept>
#include <signal.h>

#include "../mem_manager.h"


#define MEM_SIZE 10000
#define THREAD_NUMBER 124

void segfault_sigaction(int signal, siginfo_t *si, void* arg)
{
		throw std::invalid_argument("segfault_sigaction(): ");
}

void register_signal()
{
	struct sigaction sa;
	memset(&sa, 0, sizeof(struct sigaction));
	sigemptyset(&sa.sa_mask);
	sa.sa_sigaction = segfault_sigaction;
	sa.sa_flags = SA_SIGINFO;

	sigaction(SIGSEGV, &sa, NULL);
}

MemoryManager<std::mutex>* pmm = NULL;

TEST (MemoryManagerTest, Allocate)
{
		ASSERT_TRUE(pmm);
		void* block = pmm->allocate(125);

		EXPECT_TRUE(block);
		pmm->deallocate(block);
}

TEST (MemoryManagerTest, OversizeAllocate)
{
		ASSERT_TRUE(pmm);
		void* block = pmm->allocate(MEM_SIZE+125);

		EXPECT_FALSE(block);
		pmm->deallocate(block);
}

TEST (MemoryManagerTest, Deallocate)
{
		ASSERT_TRUE(pmm);
		void* block = pmm->allocate(1025);
		bool exception_triggered = false;
		try
		{
			pmm->deallocate(block);
		} catch (const std::exception& e){
			exception_triggered = true;
		}

		EXPECT_FALSE(exception_triggered);

}

TEST (MemoryManagerTest, WrongBlockDeallocate)
{
		ASSERT_TRUE(pmm);
		void* block = pmm->allocate(1025);
		bool exception_triggered = false;
		try
		{
			pmm->deallocate(reinterpret_cast<uint8_t*>(block)+1);
		} catch (const std::exception& e){
			exception_triggered = true;
		}

		EXPECT_TRUE(exception_triggered);

}

TEST (MemoryManagerTest, Alloc1Alloc2Dealloc2Dealloc1)
{
		ASSERT_TRUE(pmm);
		void* block1 = pmm->allocate(1025);
		void* block2 = pmm->allocate(1023);

		EXPECT_TRUE(block1);
		EXPECT_TRUE(block2);

		bool exception_triggered = false;
		try
		{
			pmm->deallocate(block2);
		} catch (const std::exception& e){
			exception_triggered = true;
		}

		EXPECT_FALSE(exception_triggered);

		try
		{
			pmm->deallocate(block1);
		} catch (const std::exception& e){
			exception_triggered = true;
		}

		EXPECT_FALSE(exception_triggered);
}

TEST (MemoryManagerTest, AllocDeallocAllocDealloc)
{
		ASSERT_TRUE(pmm);
		void* block1 = pmm->allocate(1025);

		EXPECT_TRUE(block1);

		bool exception_triggered = false;
		try
		{
			pmm->deallocate(block1);
		} catch (const std::exception& e){
			exception_triggered = true;
		}

		EXPECT_FALSE(exception_triggered);

		void* block2 = pmm->allocate(1023);

		EXPECT_TRUE(block2);

		try
		{
			pmm->deallocate(block2);
		} catch (const std::exception& e){
			exception_triggered = true;
		}

		EXPECT_FALSE(exception_triggered);
}

void alloc_test()
{
		void* block = pmm->allocate(65);
		ASSERT_TRUE(block);

		bool exception_triggered = false;
		try
		{
			pmm->deallocate(block);
		} catch(std::exception& e)
		{
			exception_triggered = true;
		}
		ASSERT_FALSE(exception_triggered);
}

TEST (MemoryManagerTest, ThreadAllocate)
{
	std::thread threads[THREAD_NUMBER];
	
	for (int i=0; i<THREAD_NUMBER; i++) 
	{
		threads[i] = std::thread(alloc_test);
	}

	for (int i=0; i<THREAD_NUMBER; i++)
	{
		threads[i].join();
	}

}


void run()
{
	alloc_test();
}

int main(int argc, char** argv)
{
	::testing::InitGoogleTest(&argc, argv);

	register_signal();

	uint8_t* memory = new uint8_t[MEM_SIZE];

	try
	{
		pmm = new MemoryManager<std::mutex>(memory, MEM_SIZE);
	} catch(std::exception& e)
	{
			std::cout<<"error: "<<e.what()<<std::endl;
	}

	return RUN_ALL_TESTS();

}
