#pragma once

#include "QxSemaphore.h"

template <class T>
class QxReadWriteMutex : public std::mutex
{
public:
	QxReadWriteMutex(int maxReaders = 32)
		: semaphore(maxReaders)
	{
	}

	void lockRead()
	{
		semaphore++;
	}

	void unlockRead()
	{
		semaphore--;
	}

	void lockWrite()
	{
		std::unique_lock<std::mutex> locker(*this);
		for (int i = 0; i < maxReaders(); ++i)
			semaphore++;
	}

	void unlockWrite()
	{
		semaphore -= semaphore.total();
	}

	int maxReaders() const
	{
		return semaphore.total();
	}

private:
	QxSemaphore semaphore;
};


