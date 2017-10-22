#pragma once

#include <iostream>
#include <atomic>
#include <vector>
#include <assert.h>

const std::size_t CACHE_LINE_SIZE = 128;

template <class T>
class spsc_ring_buffer 
{
public:
	//Конструктор, принимает размер циклического буфера
	spsc_ring_buffer(size_t size);

	//Операция вставки в буфер
	//Если операция проходит успешно - возращает true, если буфер переполнен - возвращает false
	bool enqueue(T e);

	//Операция извлечения из буфера
	//Если операция проходит успешно - возращает true, если буфер пуст - возвращает false
	bool dequeue(T& e);

private:

	const std::size_t _size;

	std::atomic<std::size_t> _head;

	//Второе условие паддинга: поместить _head и _tail в разные кэш-линии
	char pad_[CACHE_LINE_SIZE]; 

	std::atomic<std::size_t> _tail;

	//Структура, в которой хранится сам объект типа T, 
	//а так же массив байтов char pad[CACHE_LINE_SIZE], 
	//который гарантирует, что разные элементы T попадают в разные кэш-линии
	struct node_t 
	{
		node_t(T e) :data(e) {};
		T data;
		char pad[CACHE_LINE_SIZE];
	};

	std::vector<node_t> _data;

	size_t next(size_t current);
};

template <class T>
spsc_ring_buffer<T>::spsc_ring_buffer(size_t size) :_size(size + 1), _head(0), _tail(0), _data(size + 1, node_t(0)) 
{
	assert(_size > 1);
}

template <class T>
bool spsc_ring_buffer<T>::enqueue(T e) 
{
	size_t current_head = _head.load(std::memory_order_acquire);
	size_t current_tail = _tail.load(std::memory_order_relaxed);

	if (current_head == next(current_tail)) 
		return false;

	_data[current_tail] = node_t(e);
	current_tail = next(current_tail);

	_tail.store(current_tail, std::memory_order_release);

	return true;
}

template <class T>
bool spsc_ring_buffer<T>::dequeue(T& e) 
{
	size_t current_head = _head.load(std::memory_order_relaxed);
	size_t current_tail = _tail.load(std::memory_order_acquire);

	if (current_head == current_tail)
		return false;

	e = _data[current_head].data;

	if (current_head > 0)
		current_head--;
	else 
		current_head = _size - 1;

	_head.store(current_head, std::memory_order_release);
	return true;
}

template <class T>
std::size_t spsc_ring_buffer<T>::next(size_t current_tail)
{
	return (current_tail + 1) % _size;
}