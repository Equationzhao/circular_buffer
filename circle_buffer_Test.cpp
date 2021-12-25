// #include <gtest/gtest.h>
#include "circle_buffer.h"
#include <chrono>
#include <iostream>
#include <vector>
constexpr int N = 200;


int ctimes = 0;
int dtimes = 0;

struct data
{
	int c;
	double* b;

	data()
	{
		// std::cout<<"Create\n";
		c = 1;
		b = new double(321);
		++ctimes;
	}

	data(const data& other) : c(other.c)
	{
		b = new double(*other.b);
		++ctimes;
	}

	data(data&& other): c(other.c), b(other.b)
	{
		other.b = nullptr;
		++ctimes;
	}

	data& operator =(const data& other) noexcept
	{
		c = other.c;
		*b = *other.b;
		return *this;
	}

	data& operator =(data&& other) noexcept
	{
		c = other.c;
		delete b;
		b = other.b;
		other.b = nullptr;
		return *this;
	}

	~data()
	{
		// std::cout<<"delete\n";
		delete b;
		++dtimes;
	}
};

int main()
{
	// accurate timer
	const auto start = std::chrono::high_resolution_clock::now();
	{
		CircleBuffer<data> buffer(N);
	}

	const auto end = std::chrono::high_resolution_clock::now();

	std::cout << "costing time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
		<< "ms\n";
	std::cout << ctimes << "\n" << dtimes << "\n";
}
