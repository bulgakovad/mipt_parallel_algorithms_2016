#include "tournament_tree_mutex.h"
#include <thread>
#include <iostream>


int main()
{
	//Создаем дерево мьютексов для 8 потоков
	tree_mutex mtx(8);

	//Запускаем 8 потоков
	for (std::size_t i = 0; i != 8; i++)
	{
		mtx.lock(i);
		std::cout << "i" << std::endl;
		mtx.unlock(i);
	}

	system("pause");
    return 0;
}

