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



#pragma endregion



// template<typename T, class Allocator = std::allocator<T>>
template<typename T>
/*
    TODO(Equationzhao):
        Add concept support AND allocator support
        Add constexpr support
*/
class CircleBuffer
{
private:

	class Node
	{
	public:
		T data;
		Node *next{nullptr};
		Node *prev{nullptr};

		#pragma region Constructors && Destructor

		explicit Node(const T &data) : data(data) {}
		explicit Node(T &&data) : data(std::move(data)) {}
		Node() {}
		Node(const Node &data) = delete;
		Node(const Node &&data) = delete;
		virtual ~Node() {}

		#pragma endregion

		auto write(const T &data)
		{
			this->data = data;
		}

		auto write(T &&data)
		{
			this->data = std::move(data);
		}

		auto read()
		{
			return this->data;
		}

		// Node(const T &data, Node *next, Node *prev) : data(data), next(next), prev(prev) {}
		// Node(T &&data, Node *next, Node *prev) : data(std::move(data)), next(next), prev(prev) {}
	};

	Node *buffer;
	size_t capacity_;
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
		Node *iterator_ = buffer;
		iterator_ = new Node();

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
	 * @return true
	 * @return false
	 */
	bool destroy()
	{
		/*
		 *  Find the last node in the circular buffer
		 *  Then delete it and go back to the previous node until the head node is reached
		 */

		if (buffer == nullptr)
		{
			return true;
		}

		auto iterator_ = buffer;

		//* Recursively delete
		//! need Test!
		auto deleterBack = [=this](Node *node)
		{
			auto head_ = this;
			std::function<void(Node *)> deleter_ ;
			deleter = [&head_](Node *node)
			{
				if (node == head_->buffer)
				{
					delete node;
					return;
				}

				auto prev = node->prev;
				delete prev->next;
				return deleter(prev);
			};
		};


		// Find the last one
		if (iterator_->next != nullptr)
		{
			while (iterator_->next != buffer)
			{
				iterator_ = iterator_->next;
			}

			// Go back to the previous node and delete `next`
			//TODO(Equationzhao) Unfinished
			deleterBack(iterator_);
		}
	}

	auto getsize_() const
	{
		return size_;
	}

	auto getcapacity_() const
	{
		return capacity_;
	}

	auto gethead_() const
	{
		return head_;
	}

	auto gettail_() const
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
	CircleBuffer(CircleBuffer &&other) = delete;

	explicit CircleBuffer(const CircleBuffer &other) = delete;

	CircleBuffer &operator=(const CircleBuffer &other) = delete;

	CircleBuffer &operator=( CircleBuffer &&other) = delete;

	#pragma endregion

	#pragma region
	// TODO(Equationzhao) implementation details
	auto write() {}
	auto read() {}
	#pragma endregion


	// TODO(Equationzhao) Support user-defined deleter
	virtual ~CircleBuffer()
	{
		destroy();
	}

	[[nodiscard]]auto size() const
	{
		return size_;
	}

	[[nodiscard]]auto capacity() const
	{
		return capacity_;
	}
};






#endif // !CIRCLE_BUFFER