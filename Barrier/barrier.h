#pragma once

#include <mutex>
#include <assert.h>


class barrier
{
public:

	// онструктор барьера, принимает количество необходимых дл€ прохождени€ барьера потоков
	barrier(size_t num_threads);

	//ќсновной метод барьера
	// огда поток хочет пройти через барьер, он вызывает этот метод и блокируетс€ до тех пор, пока к барьеру не подойдут все остальные потоки.
	//ѕоток, последним вызвавший enter(), разблокирует все ожидающие потоки, после чего барьер считаетс€ пройденным.
	void enter();

private:

	std::mutex mutex_; //ћьютекс

	std::condition_variable cv_wait_for_threads; //”словна€ переменна€ с предикатом дл€ прохождени€ через барьер

	size_t num_threads; // оличество потоков, необходимых дл€ прохождени€ барьера
	size_t entered; // оличество потоков, вызвавших enter()
	size_t era; //—четчик, который инкрементируетс€, когда все потоки подошли к барьеру
};


barrier::barrier(size_t num_threads) : num_threads(num_threads), entered(0), era(0)
{
	assert(num_threads > 0);
}


void barrier::enter()
{
	std::unique_lock<std::mutex> lock(mutex_);
	entered++;
	size_t current_era = era;

	if (entered == num_threads)
	{
		entered = 0;
		era++;
		cv_wait_for_threads.notify_all();
	}
	else
	{
		cv_wait_for_threads.wait(lock, [this, current_era] { return current_era != era; });
	}
}