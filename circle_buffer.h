/**
 * @file circle_buffer.h
 * @author Equationzhao (equationzhao@foxmail.com)
 * @brief A simple concept-based C++ implementation of a circular buffer.
 * @version 0.1
 * @date 2021-12-24
 *
 * @copyright Copyright (c) 2021
 *
 */

#pragma once
#ifndef CIRCLE_BUFFER
#define CIRCLE_BUFFER

#pragma region Includes

#include <concepts>
#include <cstddef>
#include <functional>
#include <iterator>
#include <utility>
#pragma endregion


// template<typename T, class Allocator = std::allocator<T>>
template <std::copyable T>
/*
	TODO(Equationzhao):
		* Add concept Support
		* Add constexpr support
		* Use placement new instead of operator new
		? Module support
		? allocator support
		? ranges support
		
*/
class CircleBuffer
{
private:
	class Node
	{
	public:
		T data{};
		Node* next{nullptr};
		Node* prev{nullptr};

#pragma region Constructors && Destructor

		explicit Node(const T& data) : data(data)
		{
		}

		explicit Node(T&& data) : data(std::move(data))
		{
		}

		Node() = default;

		Node(const Node& data) = delete;
		Node(const Node&& data) = delete;
		auto operator=(const Node& data) = delete;
		auto operator=(Node&& data) = delete;

		virtual ~Node() = default;

#pragma endregion

		auto write(const T& data)
		{
			this->data = data;
		}

		auto write(T&& data)
		{
			this->data = std::move(data);
		}

		auto read()
		{
			return this->data;
		}
	};

	Node* buffer;
	size_t capacity_{0};
	size_t size_{0};
	size_t head_{0};
	size_t tail_{0};

	/**
	 * @brief initialize the circular buffer
	 *
	 * @param capacity_ the capacity of the buffer
	 */
	void init(size_t capacity_)
	{
		this->capacity_ = capacity_;
		this->size_ = 0;
		this->head_ = 0;
		this->tail_ = 0;

#pragma region initialize buffer
		// create buffer
		buffer = new Node();
		Node* iterator_ = buffer;

		// create nodes and link them
		for (size_t i = 0; i < capacity_ - 1; ++i)
		{
			iterator_->next = new Node();
			iterator_->next->prev = iterator_;
			iterator_ = iterator_->next;
		}

		// make it circular
		iterator_->next = buffer;
		buffer->prev = iterator_;

#pragma endregion
	}

	/**
	 * @brief destroy the circular buffer
	 *
	 */
	void destroy()
	{
		/*
		 *  Find the last node in the circular buffer
		 *  Then go back to the previous node and delete `next` until the head node is reached
		 */

		if (buffer == nullptr)
		{
			return;
		}

		auto iterator_ = buffer;

		//* Recursively delete
		//! need Test!
		//BUG: StackOverFlow
		auto deleterBack = [this](Node* node)
		{
			auto head_ = this;
			static std::function<void(Node*)> deleter_;
			deleter_ = [&head_](Node* node)
			{
				if (node == head_->buffer)
				{
					delete node;
					return;
				}

				auto prev = node->prev;
				delete prev->next;
				return deleter_(prev);
			};

			deleter_(node);
		};


		// Find the last one
		if (iterator_->next != nullptr)
		{
			while (iterator_->next != buffer)
			{
				iterator_ = iterator_->next;
			}

			// Go back to the previous node and delete `next`
			deleterBack(iterator_);
		}
	}

	auto getSize_() const
	{
		return size_;
	}

	auto getCapacity_() const
	{
		return capacity_;
	}

	auto getHead_() const
	{
		return head_;
	}

	auto getTail_() const
	{
		return tail_;
	}

public:
	// TODO(Equationzhao) impl the iterator
	// class iterator

	explicit CircleBuffer(size_t capacity_)
	{
		init(capacity_);
	}

#pragma region deleted functions
	/*
			* may implement them later
				or
			? just designed to keep them deleted
	*/
	CircleBuffer(CircleBuffer&& other) = delete;

	explicit CircleBuffer(const CircleBuffer& other) = delete;

	CircleBuffer& operator=(const CircleBuffer& other) = delete;

	CircleBuffer& operator=(CircleBuffer&& other) = delete;

#pragma endregion

#pragma region
	// TODO(Equationzhao) implementation details
	auto write()
	{
	}

	auto read()
	{
	}

	auto clear()
	{
	}

	auto sort()
	{
	}


	auto swap()
	{
	}

	auto erase()
	{
	}

	auto erase_if()
	{
	}

	[[nodiscard]] bool operator<=>(const CircleBuffer&) const
	{
	}

	[[nodiscard]] size_t size() const
	{
		return getSize_();
	}

	[[nodiscard]] size_t capacity() const
	{
		return getCapacity_();
	}

	[[nodiscard]] bool empty() const
	{
		return size == 0;
	}
#pragma endregion


	// TODO(Equationzhao) Support user-defined deleter
	virtual ~CircleBuffer()
	{
		destroy();
	}
};


#endif // !CIRCLE_BUFFER
