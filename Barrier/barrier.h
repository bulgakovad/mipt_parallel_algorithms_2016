#pragma once

#include <mutex>
#include <assert.h>


class barrier
{
public:

	//����������� �������, ��������� ���������� ����������� ��� ����������� ������� �������
	barrier(size_t num_threads);

	//�������� ����� �������
	//����� ����� ����� ������ ����� ������, �� �������� ���� ����� � ����������� �� ��� ���, ���� � ������� �� �������� ��� ��������� ������.
	//�����, ��������� ��������� enter(), ������������ ��� ��������� ������, ����� ���� ������ ��������� ����������.
	void enter();

private:

	std::mutex mutex_; //�������

	std::condition_variable cv_wait_for_threads; //�������� ���������� � ���������� ��� ����������� ����� ������

	size_t num_threads; //���������� �������, ����������� ��� ����������� �������
	size_t entered; //���������� �������, ��������� enter()
	size_t era; //�������, ������� ����������������, ����� ��� ������ ������� � �������
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