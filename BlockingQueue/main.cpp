#include "thread_safe_queue.hpp"
#include <iostream>

#define MAX_NUMBER 10000
#define CAPACITY 15
#define TASKS 100
#define CONSUMERS 3

thread_safe_queue<unsigned> tsq(CAPACITY);

//Функция проверки числа на простоту
bool check_prime(unsigned x)
{
	for (unsigned i = 2; i <= sqrt(x); i++)
	{
		if (x % i == 0)
			return false;
	}
	return true;
}

void consumer()
{
	unsigned x;

	while (true)
	{
		//Забираем число из очереди для проверки на простоту
		tsq.pop(x);
		if (x == 0)
			break;
		else
		{
			//Проверяем на простоту и выводим результат на экран
			static std::mutex mtx;
			mtx.lock();
			if (check_prime(x))
				std::cout << x << " - prime" << std::endl;
			else
				std::cout << x << " - not prime" << std::endl;
			mtx.unlock();
		}
	}
}

void producer()
{
	srand(time(0)); //для автоматической рандомизации

	//Генерируем числа, которые нужно проверить на простоту
	for (unsigned i = 0; i < TASKS; i++)
		tsq.enqueue(rand() % MAX_NUMBER + 2);

	//Кладем пилюлю с ядом, чтобы завершить consumer'а
	for (unsigned i = 0; i < CONSUMERS; i++)
		tsq.enqueue(0);

}

int main()
{
	//Запускаем producer'а и consumer'ов
	std::thread producer_(producer);
	std::vector<std::thread> consumers_(CONSUMERS);
	for (unsigned i = 0; i < CONSUMERS; i++)
		consumers_[i] = std::thread(consumer);

	//Дожидаемся завершения потоков
	producer_.join();
	for (unsigned i = 0; i < CONSUMERS; i++)
		consumers_[i].join();

	system("pause");
	return 0;
}
