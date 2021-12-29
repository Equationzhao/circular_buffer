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

int main()
{
	// accurate timer
	const auto start = std::chrono::high_resolution_clock::now();

	{
		CircularBuffer<int> buffer{100};

		std::ranges::fill(buffer, 10000);

		std::copy(buffer.begin(), buffer.end(), std::ostream_iterator<int>(std::cout, "\n "));

		print("////////////////////////////////////////////////");

		int i{};
		std::ranges::generate(buffer, [&i] { return 2 * ++i; });

		for (int data : buffer)
		{
			println(data);
		}


		print("////////////////////////////////////////////////");


		std::reverse(buffer.begin(), buffer.end()); // BUG


		for (int data : buffer)
		{
			println(data);
		}
	}

	const auto end = std::chrono::high_resolution_clock::now();

	print("costing time: ", std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count(), "ms\n");
	print("ctime: ", ctimes, "\n", "dtime: ", dtimes);
}
