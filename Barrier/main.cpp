#include "barrier.h"
#include <vector>
#include <iostream>

const size_t NUM_THREADS = 5;
const size_t NUM_TRIES = 20;

barrier barrier_(NUM_THREADS);
std::mutex mutex_;

void func()
{
	for (size_t i = 0; i < NUM_TRIES; i++)
	{
		barrier_.enter();
		std::unique_lock<std::mutex> lock(mutex_);
		std::cout << i << std::endl;
		lock.unlock();
	}
}

int main()
{
	std::vector<std::thread> threads;

	for (size_t i = 0; i < NUM_THREADS; i++)
		threads.push_back(std::thread(func));

	for (size_t i = 0; i < NUM_THREADS; i++)
		threads[i].join();
	
	
	system("pause");
    return 0;
}

