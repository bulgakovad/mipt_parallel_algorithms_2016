#include <array>
#include <atomic>
#include <thread>
#include <vector>
#include "tournament_tree_mutex.h"


peterson_mutex::peterson_mutex()
{
	want[0].store(false);
	want[1].store(false);
	victim.store(0);
}

	
//Заблокировать мьютекс Петерсона
void peterson_mutex::lock(int t)
{
	assert(t >= 0);

	want[t].store(true);
	victim.store(t);
	while (want[1 - t].load() && victim.load() == t) 
	{
		std::this_thread::yield();
	}		
}

//Разблокировать мьютекс Петерсона
void peterson_mutex::unlock(int t)
{
	assert(t >= 0);

	want[t].store(false);
}


//Конструктор tournament tree mutex, принимает количество thread'ов
tree_mutex::tree_mutex(std::size_t num_threads) : tree(num_threads - 1)
{
	assert(num_threads > 0);

	//Вычисление количества уровней дерева
	int int_lev_log = log10(num_threads) / log10(2);
	double doub_lev_log = log10(num_threads) / log10(2);

	if (int_lev_log == doub_lev_log)
		level = int_lev_log;
	else
		level = int_lev_log + 1;

	//Инициализация массива id мьютекса Петерсона потоков
	p_id.resize(num_threads);
	for (auto it = p_id.begin(); it != p_id.end(); it++)
	{
		it->resize(level);
	}
}


//Заблокировать tournament tree mutex
void tree_mutex::lock(std::size_t thread_index)
{
	assert(thread_index >= 0);

	std::size_t node_id = thread_index + tree.size() - 1;

	for (std::size_t k = 0; k <= level; k++)
	{
		p_id[thread_index][k] = node_id % 2;

		if (k >= 0)
		{
			if (node_id % 2 == 0)
				node_id = node_id / 2 - 1;
			else
				node_id = node_id / 2;
		}

		tree[node_id].lock(p_id[thread_index][k]);
	}
}

//Разблокировать tournament tree mutex
void tree_mutex::unlock(std::size_t thread_index)
{
	assert(thread_index >= 0);

	std::size_t node_id = 0;

	for (std::size_t k = level; k >= 0; k--)
	{
		tree[node_id].unlock(p_id[thread_index][k]);

		if (k > 1)
			node_id = 2 * node_id + p_id[thread_index][k] + 1;
		else
			if (k == 1)
			{
				std::size_t poss_node_id = 2 * node_id + p_id[thread_index][k] + 1;

				if (poss_node_id < tree.size() - 1)
					node_id = poss_node_id;
				else
					k = -42; // it's a kind of magic
			}
	}
}
