#include "striped_hash_set.h"
#include <iostream>
#include <thread>



int main()
{
	striped_hash_set<int> str_hash_set(4);

	std::vector<std::thread> threads;
	

	for (std::size_t i = 0; i < 10; i++)
		threads.emplace_back(std::thread([&] { str_hash_set.add(i); }));

	threads.emplace_back(std::thread([&] { std::cout << str_hash_set.contains(100500) << std::endl; }));

	for (std::size_t i = 0; i < 10; i++)
		threads.emplace_back(std::thread([&] { str_hash_set.remove(i); }));

	for (std::size_t i = 0; i < threads.size(); i++)
		threads[i].join();


	system("pause");
    return 0;
}

