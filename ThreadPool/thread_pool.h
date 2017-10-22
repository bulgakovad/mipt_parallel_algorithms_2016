#pragma once

#include "thread_safe_queue.h"
#include <functional>
#include <future>
#include <thread>



const std::size_t DEFAULT_NUM_WORKERS = 4;

template <class T>
class thread_pool
{
public:

	thread_pool();

	thread_pool(std::size_t num_workers);

	~thread_pool();

	std::shared_ptr<std::future<T> > submit(std::function<T()> func);

	void shutdown();

private:

	//Количество worker'ов
	std::size_t num_workers;

	//Массив worker'ов
	std::vector<std::thread> workers;

	//Потокобезопасная очередь задач
	//Задача представляет из себя пару из std::function (сама функция, передаваемая worker'у) 
	//и std::shared_ptr<std::promise>, с помощью которого worker опубликует результат вызова этой функции 
	thread_safe_queue<std::pair<std::function<T()>, std::shared_ptr<std::promise<T> > > > tsqueue;        

	//Метод, подбирающий необходимое число worker'ов
	void default_num_workers();
};

template <class T>
thread_pool<T>::thread_pool()
{
	//thread_safe_queue<std::pair<std::function<T()>, std::shared_ptr<std::promise<T> > > >: нет подходящего конструктора по умолчанию

	//При отсутствии указаний о количестве worker'ов подбираем их количество соответствующим методом
	default_num_workers();
	workers.resize(num_workers);
}

template <class T>
thread_pool<T>::thread_pool(std::size_t num_workers)
{
	if (num_workers >= 1)
		workers.resize(num_workers);
	else
	{
		default_num_workers();
		workers.resize(num_workers);
	}


	std::function<T()> worker_func = [this]() -> void
	{
		std::pair<std::function<T()>, std::shared_ptr<std::promise<T> > > task = tsqueue.pop();

		while (task != NULL)
		{
			(task->second)->set_value(task->first);
			task = tsqueue.pop();
		}
	};

	for (unsigned i = 0; i < num_workers; i++)
		workers.push_back(std::thread(worker_func));
}

template <class T>
thread_pool<T>::~thread_pool()
{
	shutdown();
}

template <class T>
inline std::shared_ptr<std::future<T> > thread_pool<T>::submit(std::function<T()> func)
{
	std::shared_ptr<std::promise<T> > ppromise = std::make_shared<std::promise<T>>(std::promise<T>());

	auto task = std::make_pair(func, ppromise);

	tsqueue.enqueue(task);

	return std::make_shared<std::future<T> >(ppromise->get_future());
}

template <class T>
inline void thread_pool<T>::shutdown()
{
	std::pair<std::function<T()>, std::shared_ptr<std::promise<T> > > pill = std::make_pair(nullptr, nullptr);
	for (unsigned i = 0; i < num_workers; i++)
		tsqueue.enqueue(pill);
}

template <class T>
inline void thread_pool<T>::default_num_workers()
{
	std::size_t def_num_workers;
	def_num_workers = std::thread::hardware_concurrency(); //тут происходит неявное приведение типа static unsigned к std::size_t

	if (def_num_workers == 0)
		num_workers = DEFAULT_NUM_WORKERS;
	else
		num_workers = def_num_workers;
}