#include "circular_buffer.h"
#include <algorithm>
#include <chrono>
#include <iostream>
#include <list>
#include <map>
#include <memory>
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


int main()
{
	// accurate timer
	srand(time(nullptr));
	{
		CircularBuffer<data> u(10);
		const auto start = std::chrono::high_resolution_clock::now();

		const auto times = rand();
		for (size_t i = 0; i < times; ++i)
		{
			u.insert(u.begin().operator++(), rand() % (1 + rand()));
		}

		const auto end = std::chrono::high_resolution_clock::now();
		// println(u[5]);
		println("");
		for (const auto& d : u)
		{
			println(d.x);
		}

		println("insert ", times, " times");

		println("costing time: ", std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count(), "ms\n");
		// println(data::time);
	}

	println("ctime: ", ctimes, "\n", "dtime: ", dtimes);
}
