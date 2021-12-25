// #include <gtest/gtest.h>
#include "circle_buffer.h"
#include <chrono>
#include <iostream>
#include <map>
#include <vector>
constexpr int N = 200;


int ctimes = 0;
int dtimes = 0;

struct data
{
	std::vector<std::list<uint16_t>> a;
	int c;
	double* b;
	std::unique_ptr<std::map<std::string, int>> e;


	data()
	{
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
		auto temp = *other.e;
		e = std::make_unique<std::map<std::string, int>>(temp);
		++ctimes;
	}

	data(data&& other) : a(std::move(other.a)), c(other.c), b(other.b), e(std::move(other.e))
	{
		other.b = nullptr;
		++ctimes;
	}

	data& operator =(const data& other) noexcept
	{
		a = other.a;
		c = other.c;
		*b = *other.b;
		auto temp = *other.e;
		e = std::make_unique<std::map<std::string, int>>(temp);
		return *this;
	}

	data& operator =(data&& other) noexcept
	{
		a.swap(other.a);
		c = other.c;
		delete b;
		b = other.b;
		e = std::move(other.e);
		other.b = nullptr;
		return *this;
	}

	~data()
	{
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
		// CircleBuffer<int> buffer2{10086};
		auto begin0 = buffer.begin();
		auto end0 = buffer.end();
		const auto begin1 = buffer.begin();
		const auto end1 = buffer.end();
		const auto& begin2 = buffer.begin();
		const auto& end2 = buffer.end();
		auto&& begin3 = buffer.begin();
		auto&& end3 = buffer.end();
		auto begin5 = buffer.cbegin();
		auto end5 = buffer.cend();
		const auto begin6 = buffer.cbegin();
		const auto end6 = buffer.cend();
		const auto& begin7 = buffer.cbegin();
		const auto& end7 = buffer.cend();
		auto&& begin8 = buffer.cbegin();
		auto&& end8 = buffer.cend();


		for (auto i : buffer)
		{
		}

		for (const auto& i : buffer)
		{
		}

		for (auto& i : buffer)
		{
		}

		for (auto&& i : buffer)
		{
		}

		for (auto i = buffer.begin(); i != buffer.end(); ++i)
		{
		}

		for (auto&& i = buffer.begin(); i != buffer.end(); ++i)
		{
		}
	}

	const auto end = std::chrono::high_resolution_clock::now();

	std::cout << "costing time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
		<< "ms\n";
	std::cout << ctimes << "\n" << dtimes << "\n";
}
