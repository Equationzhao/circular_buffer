// #include <gtest/gtest.h>
#include "circular_buffer.h"
#include <algorithm>
#include <chrono>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>


constexpr int N = 10;


int ctimes = 0;
int dtimes = 0;

struct data
{
	std::vector<std::list<uint16_t>> a;
	int c;
	double* b;
	std::unique_ptr<std::map<std::string, int>> e;

	data(int aa): c(aa)
	{
		// std::cout << "ctor\n";

		for (size_t i = 0; i < 100; ++i)
		{
			a.emplace_back(100);
		}

		b = new double(321);
		e = std::make_unique<std::map<std::string, int>>(std::map<std::string, int>{{"12", 1}});

		++ctimes;
	}

	data()
	{
		// std::cout << "ctor\n";

		for (size_t i = 0; i < 100; ++i)
		{
			a.emplace_back(100);
		}

		c = 1;
		b = new double(321);
		e = std::make_unique<std::map<std::string, int>>(std::map<std::string, int>{{"12", 1}});

		++ctimes;
	}

	data(const data& other) : c(other.c), b(new double(*other.b))
	{
		// std::cout << "copy\n";
		auto temp = *other.e;
		e = std::make_unique<std::map<std::string, int>>(temp);
		++ctimes;
	}

	data(data&& other) : a(std::move(other.a)), c(other.c), b(other.b), e(std::move(other.e))
	{
		// std::cout << "move\n";
		other.b = nullptr;
		++ctimes;
	}

	data& operator =(const data& other) noexcept
	{
		if (this == std::addressof(other))
		{
			return *this;
		}

		// std::cout << "copy\n";
		a = other.a;
		c = other.c;
		*b = *other.b;
		auto temp = *other.e;
		e = std::make_unique<std::map<std::string, int>>(temp);
		return *this;
	}

	data& operator =(data&& other) noexcept
	{
		if (this == std::addressof(other))
		{
			return *this;
		}

		// std::cout << "move\n";
		a = std::move(other.a);
		c = other.c;
		delete b;
		b = other.b;
		e = std::move(other.e);
		other.b = nullptr;
		return *this;
	}

	~data()
	{
		// std::cout << "des\n";
		delete b;
		++dtimes;
	}
};

int main()
{
	// accurate timer
	const auto start = std::chrono::high_resolution_clock::now();

	{
		CircularBuffer<data> buffer(N), buffer2(2 * N);


		// assert(buffer.end() - 1 + 1 == buffer.end());
		// assert(buffer.end() == buffer.begin()+buffer.capacity());

		CircularBuffer<data>::circular_iterator it(buffer.circular_begin());
		for (size_t i = 0, size = buffer.capacity(); i < 10 * size; ++i)
		{
			// std::cout << it->c;
			--it;
		}

		// for (const auto& i : buffer2)
		// {
		// 	std::cout << i.c << '\n';
		// }

		//
		//
		// for (const auto& i : buffer)
		// {
		// 	std::cout << i.c << '\n';
		// }
		//
		//
		// buffer.swap(buffer);
		//
		// for (const auto& i : buffer)
		// {
		// 	std::cout << i.c << '\n';
		// }
		// std::swap(buffer,buffer2 );
		// std::sort(buffer.begin(), buffer.end());


		// for (auto&& i : buffer)
		// {
		// 	std::cout << i.c << '\n';
		// }
	}

	const auto end = std::chrono::high_resolution_clock::now();

	std::cout << "costing time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
		<< "ms\n";
	std::cout << ctimes << "\n" << dtimes << "\n";
}
