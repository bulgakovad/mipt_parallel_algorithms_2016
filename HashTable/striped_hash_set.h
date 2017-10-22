#pragma once

#include <shared_mutex>
#include <vector>
#include <forward_list>
#include <assert.h>

const std::size_t DEFAULT_GROWTH_FACTOR = 2;
const double DEFAULT_MAX_LOAD_FACTOR = 2;

template <typename T, class H = std::hash<T>> 
class striped_hash_set
{
public:
	//Конструктор хэш-таблицы. Принимает следующие параметры:
	//num_stripes_ - число мьютексов, которые будут использоваться для lock striping
	//growth_factor - мультипликативный коэффициент роста хэш-таблицы, по умолчанию принимает значение DEFAULT_GROWTH_FACTOR
	//max_load_factor - отношение числа элементов к числу корзин, по умолчанию принимает значение DEFAULT_MAX_LOAD_FACTOR
	striped_hash_set(std::size_t num_stripes_, std::size_t growth_factor_ = DEFAULT_GROWTH_FACTOR, double max_load_factor_ = DEFAULT_MAX_LOAD_FACTOR);

	//Добавление элемента в таблицу
	void add(const T& e);

	//Удаление элемента из таблицы
	void remove(const T& e);

	//Проверка, принадлежит ли элемент e множеству
	bool contains(const T& e);

	//Явно запретим конструктор по умолчанию, конструктор копирования и оператор присваивания
	striped_hash_set() = delete;
	striped_hash_set(const striped_hash_set &other) = delete;
	striped_hash_set &operator=(const striped_hash_set &other) = delete;

private:
	std::vector<std::forward_list<T> > hash_table; //Сама хэш-таблица
	
	//unique_lock - блокировка на запись, shared_lock - блокировка на чтение
	std::vector<std::shared_timed_mutex> mutexes; //Массив мьютексов фиксированного размера

	std::size_t num_stripes; //Число мьютексов, которые будут использоваться для lock striping
	std::size_t growth_factor; //Мультипликативный коэффициент роста хэш-таблицы
	double max_load_factor; //Отношение числа элементов к числу корзин, т.е. среднее число элементов на корзину
	std::size_t num_elems;
	std::mutex num_elems_mtx;

};

template <typename T, class H>
striped_hash_set<T, H>::striped_hash_set(std::size_t num_stripes_, std::size_t growth_factor_ , double max_load_factor_) :
	num_stripes(num_stripes_), growth_factor(growth_factor_), max_load_factor(max_load_factor_), mutexes(num_stripes_), hash_table(num_stripes_), num_elems(0)
{
	assert(num_stripes > 0);
	assert(growth_factor > 1);
	assert(max_load_factor > 0);
}

template <typename T, class H>
void striped_hash_set<T, H>::add(const T& e)
{
	std::size_t num_baskets = hash_table.size();
	//Вычисляем, какому страйпу принадлежит элемент, по формуле (h(x) mod m) mod n, где h(x) хэш-функция, m - кол-во корзин, n - кол-во страйпов
	std::size_t num_stripe = (H()(e) % num_baskets) % num_stripes;

	//Заблокируем подходящий мьютекс на чтение
	std::shared_lock<std::shared_timed_mutex> lock(mutexes[num_stripe]);

	//Проверяем, есть ли элемент в таблице и, если нет, добавляем его
	if (!contains(e))
	{
		lock.unlock();
		//Заблокируем подходящий мьютекс на запись
		std::unique_lock<std::shared_timed_mutex> lock2(mutexes[num_stripe]);

		//Вычисляем, в какую корзину попал элемент, по формуле h(x) mod m, где h(x) хэш-функция, m - кол-во корзин
		std::size_t num_basket = H()(e) % num_baskets;
		//Добавляем его
		hash_table[num_basket].push_front(e);

		//Увеличиваем счетчик количества элементов в таблице
		num_elems_mtx.lock();
		num_elems++;
		num_elems_mtx.unlock();


		//Проверяем, нужно ли теперь расширить таблицу и, если нужно, расширяем
		double load_factor = double(num_elems)/num_baskets;

		if (load_factor > max_load_factor)
		{
			std::size_t curr_num_baskets  = num_baskets;
			std::vector<std::unique_lock<std::shared_timed_mutex> > locks;
			//Заблокируем первый мьютекс на запись
			locks.emplace_back(mutexes[0]);

			//Проверим, не расширили ли таблицу раньше
			if (curr_num_baskets == num_baskets)
			{
				//Блокируем все мьютексы по порядку
				for (std::size_t i = 0; i < mutexes.size(); i++)
					locks.emplace_back(mutexes[i]);

				//Создаем новую хэш-таблицу и копируем туда все элементы
				std::vector<std::forward_list<T> > new_hash_table(curr_num_baskets*growth_factor);
				for (auto it = hash_table.begin(); it != hash_table.end(); it++)
					for (auto it2 = it->begin(); it2 != it->end(); it2++)
						new_hash_table[H()(*it2) % curr_num_baskets].push_front(*it2);

				hash_table = new_hash_table;
			}
		}
	}
}

template <typename T, class H>
void striped_hash_set<T, H>::remove(const T& e)
{
	std::size_t num_baskets = hash_table.size();
	//Вычисляем, какому страйпу принадлежит элемент, по формуле (h(x) mod m) mod n, где h(x) хэш-функция, m - кол-во корзин, n - кол-во страйпов
	std::size_t num_stripe = (H()(e) % num_baskets) % num_stripes;

	//Заблокируем подходящий мьютекс на чтение
	std::shared_lock<std::shared_timed_mutex> lock(mutexes[num_stripe]);

	//Проверяем, есть ли элемент в таблице и, если есть, удаляем его
	if (contains(e))
	{
		lock.unlock();
		//Заблокируем подходящий мьютекс на запись
		std::unique_lock<std::shared_timed_mutex> lock2(mutexes[num_stripe]);

		//Вычисляем, в какую корзину попал элемент, по формуле h(x) mod m, где h(x) хэш-функция, m - кол-во корзин
		std::size_t num_basket = H()(e) % hash_table.size();
		//Удаляем его
		hash_table[num_basket].remove(e);

		//Уменьшаем счетчик количества элементов в таблице
		num_elems_mtx.lock();
		num_elems--;
		num_elems_mtx.unlock();
	}
}

template <typename T, class H>
bool striped_hash_set<T, H>::contains(const T& e)
{
	//Вычисляем, в какую корзину попал элемент, по формуле h(x) mod m, где h(x) хэш-функция, m - кол-во корзин
	std::size_t num_basket = H()(e) % hash_table.size();
	//Проходимся по корзине в поисках элемента
	for (auto it = hash_table[num_basket].begin(); it != hash_table[num_basket].end(); it++)
		if (*it == e)
			return true;

	return false;
}