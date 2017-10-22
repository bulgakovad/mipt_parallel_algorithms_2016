#pragma once
#include <array>
#include <atomic>
#include <thread>
#include <vector>
#include <cassert>

class peterson_mutex {
public:
	peterson_mutex();

	//Заблокировать мьютекс Петерсона
	void lock(int t);

	//Разблокировать мьютекс Петерсона
	void unlock(int t);

private:
	std::array<std::atomic<bool>, 2> want;
	std::atomic<int> victim;
};


class tree_mutex {
public:

	tree_mutex(std::size_t num_threads);

	//Заблокировать tournament tree mutex
	void lock(std::size_t thread_index);

	//Разблокировать tournament tree mutex
	void unlock(std::size_t thread_index);

private:
	std::vector<peterson_mutex> tree; //Дерево мьютексов Петерсона

	std::size_t level; //Количество уровней дерева
	std::vector<std::vector<bool> > p_id; //Массив id мьютекса Петерсона потоков
};
