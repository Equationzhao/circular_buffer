// #include <gtest/gtest.h>
#include "circular_buffer.h"
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
	double *b;
	std::unique_ptr<std::map<std::string, int>> e;

	data(int aa): c(aa)
	{
		std::cout << "ctor\n";

		for (size_t i = 0; i < 100; ++i)
		{
			a.emplace_back(100);
		}

		b = new double(321);
		e = std::make_unique<std::map<std::string, int>>(std::map<std::string, int> {{"12", 1}});

		++ctimes;
	}

	data()
	{
		std::cout << "ctor\n";

		for (size_t i = 0; i < 100; ++i)
		{
			a.emplace_back(100);
		}

		c = 1;
		b = new double(321);
		e = std::make_unique<std::map<std::string, int>>(std::map<std::string, int> {{"12", 1}});

		++ctimes;
	}

	data(const data &other) : c(other.c), b(new double(*other.b))
	{
		std::cout << "copy\n";
		auto temp = *other.e;
		e = std::make_unique<std::map<std::string, int>>(temp);
		++ctimes;
	}

	data(data &&other) : a(std::move(other.a)), c(other.c), b(other.b), e(std::move(other.e))
	{
		std::cout << "move\n";
		other.b = nullptr;
		++ctimes;
	}

	data &operator =(const data &other) noexcept
	{
		std::cout << "copy\n";
		a = other.a;
		c = other.c;
		*b = *other.b;
		auto temp = *other.e;
		e = std::make_unique<std::map<std::string, int>>(temp);
		return *this;
	}

	data &operator =(data &&other) noexcept
	{
		std::cout << "move\n";
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
		std::cout << "des\n";
		delete b;
		++dtimes;
	}
};

int main()
{
	// accurate timer
	const auto start = std::chrono::high_resolution_clock::now();

	{
		// std::list<data> list;
		// for (int i = 0; i < N; ++i)
		// {
		// 	list.emplace_back(data());
		// }
		CircularBuffer<data> buffer(N), buffer2(2 * N);

		// CircularBuffer<int> buffer2{10086};
		// auto begin0 = buffer.begin();
		// auto end0 = buffer.end();
		// const auto begin1 = buffer.begin();
		// const auto end1 = buffer.end();
		// const auto& begin2 = buffer.begin();
		// const auto& end2 = buffer.end();
		// auto&& begin3 = buffer.begin();
		// auto&& end3 = buffer.end();
		// auto begin5 = buffer.cbegin();
		// auto end5 = buffer.cend();
		// const auto begin6 = buffer.cbegin();
		// const auto end6 = buffer.cend();
		// const auto& begin7 = buffer.cbegin();
		// const auto& end7 = buffer.cend();
		// auto&& begin8 = buffer.cbegin();
		// auto&& end8 = buffer.cend();


		// for (size_t i = 0, end = buffer.capacity(); i < end; ++i)
		// {
		// 	buffer.write(i);
		// }
		//
		// for (size_t i = 0, end = buffer2.capacity(); i < end; ++i)
		// {
		// 	buffer2.write(i);
		// }
		//
		//
		// for (auto i : buffer)
		// {
		// 	std::cout << i.c << '\n';
		// }
		//
		//
		// buffer.swap(buffer);
		//
		// for (auto i : buffer)
		// {
		// 	std::cout << i.c << '\n';
		// }


		//
		// for (const auto& i : buffer)
		// {
		// 	std::cout << i.c << '\n';
		// }

		// for (auto& i : buffer)
		// {
		// 	std::cout << i.c << '\n';
		// }
		//
		// for (auto&& i : buffer)
		// {
		// 	std::cout << i.c << '\n';
		// }
		//
		// for (auto i = buffer.begin(); i != buffer.end(); ++i)
		// {
		// 	std::cout << i->c << '\n';
		// }
		//
		// for (auto&& i = buffer.begin(); i != buffer.end(); ++i)
		// {
		// 	std::cout << i->c << '\n';
		// }

		// for (size_t i = 0, end = buffer.capacity(); i < end; ++i)
		// {
		// 	std::cout << buffer.read().c << '\n';
		// }

		// for (size_t i = 0, end = buffer.capacity(); i < end; ++i)
		// {
		// 	std::cout << buffer[i].c << '\n';
		// }

		data c(100);

		for (size_t i = 0, end = buffer.capacity(); i < end; ++i)
		{
			buffer.write(std::move(c));
		}

		//
		// for (const auto& data : buffer)
		// {
		// 	std::cout << data.c << '\n';
		// }
	}

	const auto end = std::chrono::high_resolution_clock::now();

	std::cout << "costing time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
	          << "ms\n";
	std::cout << ctimes << "\n" << dtimes << "\n";
}
