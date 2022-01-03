#include "circular_buffer.h"
#include <algorithm>
#include <chrono>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <numeric>
#include <random>
#include <string>
#include <vector>
#include <fast_io/fast_io.h>
// import Circular_Buffer;
constexpr int N = 5000;


int ctimes = 0;
int dtimes = 0;


template <class T, class... U>
constexpr auto make_obj(U&& ... t)
{
	return T(std::forward<U>(t)...);
}


struct data
{
	int x;
	inline static int time{0};

	data() : x(0)
	{
		++time;
	}

	data(int x): x(x)
	{
		++time;
	}

	data(const data& other): x(other.x)
	{
		++time;
	}
};


auto timer_ms(const std::function<void()> a)
{
	const auto start = std::chrono::high_resolution_clock::now();

	a();

	const auto end = std::chrono::high_resolution_clock::now();

	return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}


int main()
{
	constexpr int step = 3;
	constexpr int number = 10;
	CircularBuffer<int> circle(number);

	std::iota(circle.begin(), circle.end(), 1);

	int res;
	int i{1};
	auto it = circle.circular_begin();
	for (;;)
	{
		if (i == step)
		{
			i = 1;
			it = circle.erase(it, it + 1);
			if (circle.capacity() == 1)
			{
				res = *it;
				break;
			}
		}
		else
		{
			++i;
			++it;
		}
	}

	println(res);
}
