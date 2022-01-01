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


auto timer_ms(const std::function<void()> a)
{
	const auto start = std::chrono::high_resolution_clock::now();

	a();

	const auto end = std::chrono::high_resolution_clock::now();

	return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}


int main(){
	{
		CircularBuffer<data> u(10);
		auto i{0};
		std::ranges::generate(u, [&i] { return ++i; });
		println((*(std::begin(u) + 3)).x);
	}
}
