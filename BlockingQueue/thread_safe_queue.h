#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>

template<class T>
class thread_safe_queue
{
public:

	//Конструктор, принимает размер блокирующей очереди
	thread_safe_queue(std::size_t capacity_);

	//Конструктор копирования и оператор присваивания явно запрещены, так как не имеют смысла
	thread_safe_queue(thread_safe_queue &tsq) = delete;
	thread_safe_queue &operator=(thread_safe_queue &tsq) = delete;

	//Операция вставки в очередь
	void enqueue(const T& item);

	//Операция удаления из очереди 
	void pop(T& item);


private:

	//Очередь и ее размер
	std::queue<T> thread_queue;
	std::size_t capacity;

	//Мьютекс
	std::mutex mutex_;

	//Условные переменные для блокировки на пустой/полной очереди
	std::condition_variable cv_empty;
	std::condition_variable cv_overflow;
};

template<class T>
inline thread_safe_queue<T>::thread_safe_queue(std::size_t capacity_) :capacity(capacity_) {}

template<class T>
inline void thread_safe_queue<T>::enqueue(const T& item)
{
	std::unique_lock<std::mutex> lock(mutex_);

	cv_overflow.wait(lock, [this] { return thread_queue.size() < capacity; });

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