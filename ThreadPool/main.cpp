#include "thread_pool.h"
#include<ctime>
#include <iostream>

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


int main()
{
	std::vector<int> numbers;
	std::vector<std::shared_ptr<std::future<bool> > > results;
	thread_pool<bool> thr_p;

	srand(std::time(0)); //для автоматической рандомизации
	
	for (unsigned i = 0; i < 10; i++)
	{
		numbers.push_back(rand() % 10000 + 2);
		results.push_back(thr_p.submit(std::bind(check_prime, numbers[i])));
	}

	for (unsigned i = 0; i < 10; i++)
		std::cout << numbers[i] << (results[i].get() ? " is  prime" : " is not prime") << std::endl;


	system("pause");
	return 0;
}