#pragma once
#include <mutex>
#include <condition_variable>

namespace net
{

class CountDownLatch
{
public:
	CountDownLatch(const CountDownLatch&) = delete;
	CountDownLatch& operator = (const CountDownLatch&) = delete;

	explicit CountDownLatch(int n):
		count_(n)
	{
	}

	void wait()
	{
		std::unique_lock<std::mutex> lock(mutex_);
		while (count_ > 0)
		{
			cond_.wait(lock);
		}
	}

	void countDown()
	{
		std::unique_lock<std::mutex> lock(mutex_);
		count_--;

		if (count_ == 0)
		{
			cond_.notify_one();
		}
	}

	int getCount() const
	{
		std::unique_lock<std::mutex> lock(mutex_);
		return count_;
	}

	void reset(int n)
	{
		std::unique_lock<std::mutex> lock(mutex_);
		count_ = n;
	}

private:
	std::condition_variable cond_;
	int count_;
	mutable std::mutex mutex_;
};

}