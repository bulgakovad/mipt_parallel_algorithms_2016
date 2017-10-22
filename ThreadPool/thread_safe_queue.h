#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>

template<class T>
class thread_safe_queue
{
public:

	thread_safe_queue();

	//Конструктор копирования и оператор присваивания явно запрещены, так как не имеют смысла
	thread_safe_queue(thread_safe_queue &tsq) = delete;
	thread_safe_queue &operator=(thread_safe_queue &tsq) = delete;

	//Операция вставки в очередь
	void enqueue(const T& item);

	//Операция удаления из очереди 
	void pop(T& item);


private:

	//Очередь
	std::queue<T> thread_queue;

	//Мьютекс
	std::mutex mutex_;

	//Условная переменная для блокировки на пустой очереди
	std::condition_variable cv_empty;
};

template<class T>
inline thread_safe_queue<T>::thread_safe_queue() {}



template<class T>
inline void thread_safe_queue<T>::enqueue(const T& item)
{
	std::unique_lock<std::mutex> lock(mutex_);

	thread_queue.push(item);

	cv_empty.notify_one();
}

template<class T>
inline void thread_safe_queue<T>::pop(T& item)
{
	std::unique_lock<std::mutex> lock(mutex_);

	cv_empty.wait(lock, [this] { return !thread_queue.empty(); });

	item = thread_queue.front();
	thread_queue.pop();

	cv_overflow.notify_one();
}