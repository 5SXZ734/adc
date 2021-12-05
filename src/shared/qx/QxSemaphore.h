#pragma once

#include <mutex>
#include <condition_variable>

#ifdef max
#undef max
#endif


class QxSemaphore
{
public:
	QxSemaphore(int maxcount)
		: value(0), max(maxcount)
	{
	}

	virtual ~QxSemaphore()
	{
	}

	int operator++(int)
	{
		std::unique_lock<std::mutex> locker(mutex);
		while (value >= max)
			cond.wait(locker);

		++value;
		if (value > max)
			value = max;

		return value;
	}

	int operator--(int)
	{
		std::unique_lock<std::mutex> locker(mutex);

		--value;
		if (value < 0)
			value = 0;

		cond.notify_all();

		return value;
	}


	int operator+=(int n)
	{
		std::unique_lock<std::mutex> locker(mutex);

		if (n < 0 || n > max) {
			n = n < 0 ? 0 : max;
		}

		while (value + n > max)
			cond.wait(locker);

		value += n;

		return value;
	}


	int operator-=(int n)
	{
		std::unique_lock<std::mutex> locker(mutex);

		if (n < 0 || n > value) {
			n = n < 0 ? 0 : value;
		}

		value -= n;
		cond.notify_all();

		return value;
	}


	int available() const
	{
		std::unique_lock<std::mutex> locker(const_cast<std::mutex&>(mutex));
		return max - value;
	}


	int total() const
	{
		std::unique_lock<std::mutex> locker(const_cast<std::mutex&>(mutex));
		return max;
	}


	bool tryAccess(int n)
	{
		std::unique_lock<std::mutex> locker(mutex);

		if (value + n > max)
			return false;

		value += n;

		return true;
	}


private:
	std::mutex mutex;
	std::condition_variable cond;
	int value, max;

private:
    QxSemaphore(const QxSemaphore &);
    QxSemaphore &operator=(const QxSemaphore &);
};


