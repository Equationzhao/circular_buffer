/**
 * @file circular_buffer.h
 * @author Equationzhao (equationzhao@foxmail.com)
 * @brief A simple concept-based C++ implementation of a circular buffer.
 * @version 0.1
 * @date 2021-12-24
 *
 * @copyright Copyright (c) 2021
 *
 */

#pragma once
#ifndef CIRCULAR_BUFFER
#define CIRCULAR_BUFFER


#pragma region Includes
#include <cassert>
#include <concepts>
#include <cstddef>
#include <functional>
#include <iterator>
#include <new>
#include <stdexcept>
#include <utility>

#pragma endregion

template <typename T>
using raw_ptr = T*;

template <typename T>
using observer_ptr = raw_ptr<T>; // to const?

template <typename T>
constexpr auto make_observer(T& obj)
{
	return std::addressof(obj);
}

template <typename T>
constexpr observer_ptr<T> make_observer(T* obj)
{
	return obj;
}


template <std::copyable T, class Alloc = std::allocator<T>>
/*
	TODO(Equationzhao):
		* Add concept Support
		* Add constexpr support
		* Add user-defined comparator support
		* Add user-defined deleter support
		? Module support
*/
class CircularBuffer
{
private:
	/**
	 * @brief Node
	 *		contains the data
	 *		contains the last and next observer_ptr<Node>
	 */
	class Node
	{
	private:
		constexpr static auto one{1};
	public:
		using self = Node;
		using value_type = T;
#pragma region data and pointer
		raw_ptr<value_type> data{nullptr};
		observer_ptr<self> next{nullptr};
		observer_ptr<self> prev{nullptr};

		size_t distance{0};
#pragma endregion

#pragma region Constructors && Destructor

		// constexpr explicit Node(const value_type& data)
		// {
		// 	make_obj(data);
		// }
		//
		// constexpr explicit Node(value_type&& data)
		// {
		// 	make_obj(std::move(data));
		// }

		// constexpr Node()
		// {
		// 	make_obj();
		// }

		template <typename ...Args>
		explicit constexpr Node(Args&&... args)
		{
			make_obj(std::forward<Args>(args)...);
		}

		constexpr Node(const Node& other) : next(other.next), prev(other.prev),
											distance(other.distance)
		{
			make_obj(other.data);
		}

		constexpr Node(Node&& other) noexcept: next(other.next), prev(other.prev),
											   distance(other.distance)

		{
			make_obj(std::exchange(other.data, nullptr));
		}

		constexpr Node& operator=(const Node& other)
		{
			if (this == std::addressof(other))
			{
				return *this;
			}
			*data = *other.data;
			next = other.next;
			prev = other.prev;
			distance = other.distance;

			return *this;
		}

		constexpr Node& operator=(Node&& other) noexcept
		{
			data = std::exchange(other.data, nullptr);
			next = other.next;
			prev = other.prev;
			distance = other.distance;

			return *this;
		}

		// constexpr auto make_obj()
		// {
		// 	data = alloc_.allocate(one);
		// 	std::construct_at(data);
		// }

		template <typename ... Args>
		constexpr auto make_obj(Args&&... args)
		{
			data = alloc_.allocate(one);
			std::construct_at(data, std::forward<Args>(args)...);
		}

		// constexpr auto make_obj(const value_type& obj)
		// {
		// 	data = alloc_.allocate(one);
		// 	std::construct_at(data, obj);
		// }

		// constexpr auto make_obj(value_type&& obj)
		// {
		// 	data = alloc_.allocate(one);
		// 	std::construct_at(data, std::move(obj));
		// }

		constexpr auto destroy_obj() noexcept
		{
			if (data != nullptr)
			{
				std::destroy_at(data);
			}
			alloc_.deallocate(data, 1);
		}

		constexpr virtual ~Node()
		{
			destroy_obj();
		}

		// ? TODO compare the T data it contains
		// or  test `are they actually the same object`
		constexpr bool operator==(const Node& rhs) const
		{
			return *(this->data) == *(rhs.data);
		}

		constexpr auto operator<=>(const Node& rhs) const
		{
			return *(this->data) <=> *(rhs.data);
		}

		constexpr auto swap(Node& other) noexcept
		{
			const auto temp = other;
			other = *this;
			*this = temp;
		}

#pragma endregion

#pragma region Modifiers
		/*
		 * @brief write data
		 */
		// auto write(const value_type& dataToWrite)
		// {
		// 	this->data = make_obj(dataToWrite);
		// }

		/*
		 * @brief write data
		 */
		template <typename ...Args>
		auto write(Args&& ...dataToWrite)
		{
			make_obj(std::forward<Args>(dataToWrite)...);
		}

		/*
		 * @brief get a copy of the data
		 * @return value
		 */
		[[nodiscard]] value_type read() const
		{
			return *(this->data);
		}


		/**
		 * @brief  get Reference of the data
		 * @return reference
		 */
		[[nodiscard]] value_type& get() noexcept
		{
			return *(this->data);
		}

		/**
		 * @brief  get Reference of the data
		 * @return reference
		 */
		[[nodiscard]] value_type& get() const
		{
			return *(this->data);
		}


		/**
		 * @brief  get const-reference of the data
		 * @return  const reference
		 */
		[[nodiscard]] const value_type& const_get() const
		{
			return *(this->data);
		}


#pragma endregion
	};


	/*
	 * @exchange the contained value
	 *	attention to self-assign
	 */
	static void exchangeNode(raw_ptr<Node> lhs, raw_ptr<Node> rhs)
	{
		if (lhs == rhs)
		[[unlikely]]
		{
			return;
		}

		std::swap(lhs->data, rhs->data);
	}


	/*
	 * @brief if index <= capacity/2 go forward
	 *		  else go backward
	 */
	raw_ptr<Node> findNode(const size_t index)
	{
		if (index == 0)
		[[unlikely]]
		{
			return buffer_;
		}
		assert(index < capacity_);
		auto iterator_ = make_observer(buffer_);

		if (index <= capacity_ / 2) // forward
		{
			for (size_t i = 0; i < index; ++i)
			{
				iterator_ = iterator_->next;
			}
		}
		else // backward
		{
			for (size_t i = 0; i < capacity_ - index; ++i)
			{
				iterator_ = iterator_->prev;
			}
		}

		return iterator_;
	}


	/*
	 *@brief
	 *	Before insert
	 *		...->a->where->...
	 *	After  insert
	 *		...->a->toInsert->where->...
	 *
	 *
	 *	//TODO FIXME update distance
	 */
	void insertNode(raw_ptr<Node> where, raw_ptr<Node> toInsert)
	{
		auto& prePtr = where->prev;
		auto& prev = *(where->prev);


		prePtr = toInsert;
		toInsert->next = make_observer(where);


		prev.next = make_observer(toInsert);
		toInsert->prev = make_observer(prev);


		toInsert->distance = 1 + prev.distance;
		auto iterator_ = toInsert->next;

		do
		{
			++iterator_->distance;
			iterator_ = iterator_->next;
		}
		while (iterator_ != buffer_);

		++capacity_;
	}

	/*
	 * @brief insert between where and where->next
	 */
	void insertNodeAfter(raw_ptr<Node> where, raw_ptr<Node> toInsert)
	{
		auto& nextPtr = where->next;
		auto& next = *(where->next);


		nextPtr = toInsert;
		toInsert->prev = make_observer(where);


		next.prev = make_observer(toInsert);
		toInsert->next = make_observer(next);


		toInsert->distance = 1 + where->distance;
		auto iterator_ = toInsert->next;

		while (iterator_ != buffer_)
		{
			++iterator_->distance;
			iterator_ = iterator_->next;
		}

		++capacity_;
	}


	template <typename ...Args>
	void insertNode(raw_ptr<Node> where, Args&& ...args)
	{
		auto toInsert = make_node(std::forward<Args>(args)...);
		insertNode(where, toInsert);
	}

	template <typename ...Args>
	void insertNodeAfter(raw_ptr<Node> where, Args&& ...args)
	{
		auto toInsert = make_node(std::forward<Args>(args)...);
		insertNodeAfter(where, toInsert);
	}


	template <typename ...Args>
	static raw_ptr<Node> make_node(Args&& ...args)
	{
		return std::construct_at(nodeAlloc_.allocate(1), std::forward<Args>(args)...);
	}

	/*
	 * @brief destruct and
	 *		free the memory
	 */
	auto destroyNode(raw_ptr<Node> which)
	{
		std::destroy_at(which);
		nodeAlloc_.deallocate(which);
	}


	/*
	 * @brief delete the node from the circle
	 *		link the previous one and the next one
	 *		the destroy the node
	 */
	void deleteNode(raw_ptr<Node> which)
	{
		auto& preNode = which->prev;
		auto& nextNode = which->next;

		preNode->next = make_observer(nextNode);
		nextNode->prev = make_observer(preNode);

		destroyNode(which);

		auto iterator_ = nextNode;
		do
		{
			--(iterator_->distance);
			iterator_ = iterator_->next;
		}
		while (iterator_ != buffer_);
	}


	inline static Alloc alloc_{};
	inline static std::allocator<Node> nodeAlloc_{};

private:
	raw_ptr<Node> buffer_{nullptr};
	size_t capacity_{0};
	size_t size_{0};

	observer_ptr<Node> toWrite;
	observer_ptr<Node> toRead;

	/**
	 * @brief initialize the circular buffer
	 *
	 * @param capacityToInit the capacity of the buffer
	 */
	constexpr void init(const size_t capacityToInit)
	{
		this->capacity_ = capacityToInit;
		this->size_ = 0;


#pragma region initialize buffer
		// create buffer
		buffer_ = new Node();
		observer_ptr<Node> iterator_ = make_observer(buffer_);


		// create nodes and link them
		// update distance
		size_t distance{0};
		iterator_->distance = distance;

		for (size_t i = 0; i < capacityToInit - 1; ++i)
		{
			iterator_->next = make_node();
			iterator_->next->prev = iterator_;
			iterator_ = iterator_->next;
			iterator_->distance = ++distance;
		}

		// make it circular
		iterator_->next = make_observer(buffer_);
		buffer_->prev = make_observer(iterator_);

		toWrite = make_observer(buffer_);
		toRead = make_observer(buffer_);

#pragma endregion
	}

	//////////////////////////////////////////////////////////
	// auto deleter_(observer_ptr<Node> node)				//
	// {													//
	// 	if (node == this->buffer_)							//
	// 	{													//
	// 		delete node;									//
	// 		return;											//
	// 	}				"Thank you, StackOverFlow"			//
	//														//
	// 		auto prev = node->prev;							//
	// 		delete prev->next;								//
	// 		return deleter_(prev);							//
	// }													//
	//														//
	// void recursivelyDelete(observer_ptr<Node> node)		//
	// {													//
	// 		deleter_(node);									//
	// }													//
	//////////////////////////////////////////////////////////

	/**
	 * @brief destroy the circular buffer
	 *
	 */
	constexpr void destroy()
	{
		/*
		 *  Find the last node in the circular buffer
		 *  Then go back to the previous node and delete `next` until the head node is reached
		 */

		if (buffer_ == nullptr)
		{
			return;
		}

		auto iterator_ = this->buffer_->prev;
		for (size_t i = 0; i < capacity_; ++i)
		{
			if (iterator_ == buffer_)
			{
				delete iterator_;
				return;
			}
			iterator_ = iterator_->prev;
			delete iterator_->next;
		}
	}

#pragma region getters

	[[nodiscard]] auto getSize_() const
	{
		return size_;
	}

	[[nodiscard]] auto getCapacity_() const
	{
		return capacity_;
	}

	[[nodiscard]] auto getHeadElement()
	{
		return buffer_->get();
	}

	[[nodiscard]] auto getTailElement()
	{
		return buffer_->prev->get();
	}

	[[nodiscard]] auto getHeadElement() const
	{
		return buffer_->const_get();
	}

	[[nodiscard]] auto getTailElement() const
	{
		return buffer_->prev->const_get();
	}

	[[nodiscard]] auto getPtrHead_()
	{
		return std::addressof(buffer_->data);
	}

	[[nodiscard]] auto getPtrTail_()
	{
		return std::addressof(buffer_->prev->data);
	}


#pragma endregion

public:
#pragma region Iterators Defination

	/*
	* @brief normal iterator
	*/
	class iterator
	{
	public:
		using iterator_concept = std::bidirectional_iterator_tag;
		using iterator_category = std::bidirectional_iterator_tag;
		using value_type = Node;
		using difference_type = ptrdiff_t;
		using pointer = observer_ptr<T>;
		using reference = T&;
		using self = iterator;
		using const_pointer = const observer_ptr<T>;
		using const_reference = const T&;
		using container_ptr = observer_ptr<CircularBuffer>;
		using const_container_ptr = const observer_ptr<CircularBuffer>;
	private:
		observer_ptr<value_type> ptr_;
		const_container_ptr proxy_;

		auto clone(const iterator& other)
		{
			ptr_ = other.ptr_;
			const_cast<container_ptr&>(proxy_) = other.proxy_;
		}

	protected:
		[[nodiscard]] auto getProxy_() const
		{
			return proxy_;
		}


	public:
		friend class CircularBuffer;

		iterator() : ptr_(nullptr), proxy_(nullptr)
		{
		}

		constexpr iterator(observer_ptr<value_type> ptr, const_container_ptr proxy) noexcept : ptr_(ptr), proxy_(proxy)
		{
		}

		constexpr iterator(const iterator& other) : ptr_(other.ptr_), proxy_(other.proxy_)
		{
		}

		iterator(iterator&& other) noexcept : ptr_(other.ptr_), proxy_(other.proxy_)
		{
		}

		iterator& operator =(const iterator& other)
		{
			if (this == std::addressof(other))
			[[unlikely]]
			{
				return *this;
			}

			clone(other);
			return *this;
		}

		constexpr iterator& operator =(iterator&& other) noexcept
		{
			clone(other);
			return *this;
		}

		constexpr bool operator==(const iterator& other) const
		{
			// When compare `end` to `end`,
			// if they are the iterators of the some container
			// return true, else return false
			return ptr_ == other.ptr_ && proxy_ == other.proxy_;
		}

		constexpr reference operator*()
		{
			assert(this->ptr_ != nullptr);

			return ptr_->get();
		}

		constexpr const_reference operator*() const
		{
			assert(this->ptr_ != nullptr);

			return ptr_->const_get();
		}

		constexpr pointer operator->() const
		{
			assert(this->ptr_ != nullptr);

			return std::addressof(ptr_->data);
		}

		constexpr self& operator++()
		{
			assert(ptr_ != nullptr);

			if (ptr_ == proxy_->buffer_->prev)
			[[unlikely]]
			{
				ptr_ = nullptr;
			}
			else
			[[likely]]
			{
				ptr_ = ptr_->next;
			}

			return *this;
		}


		constexpr self operator++(int) &
		{
			self tmp = *this;

			++(*this);

			return tmp;
		}

		// ! need test
		// When it's end, continue to + will call abort
		constexpr self operator+(difference_type n) const;

		// ! need test
		constexpr self& operator+=(difference_type n)
		{
			// assert(ptr_ != nullptr);

			if (n == 0)
			[[unlikely]]
			{
				return *this;
			}

			if (n < 0)
			[[unlikely]]
			{
				return (*this -= -n);
			}

			for (difference_type i = 0; i < n; ++i)
			{
				++(*this);
			}

			return *this;
		}

		// When it's begin(), -- will call abort
		constexpr self& operator--()
		{
			assert(this->ptr_ == proxy_->buffer_);
			if (this->ptr_ == nullptr)
			[[unlikely]]
			{
				this->ptr_ = proxy_->buffer_->prev;
				return *this;
			}


			ptr_ = ptr_->prev;

			return *this;
		}

		// ! need test
		// rely on the implementation of prefix--
		// when n>distance it will continue self-sub from the beginning
		constexpr self operator--(int) &
		{
			self tmp = *this;
			--(*this);
			return tmp;
		}

		// ! need test
		constexpr self operator-(difference_type n) const;

		// ! need test
		// May be BUG
		// pay attention to begin & end
		constexpr difference_type operator-(const iterator& rhs) const
		{
			assert(this->proxy_ == rhs.proxy_);

			const auto iterator_ = this->ptr_;

			if (iterator_ == nullptr)
			[[unlikely]]
			{
				if (rhs.ptr_ == nullptr)
				[[unlikely]]
				{
					return 0;
				}
				else
				{
					return static_cast<difference_type>(proxy_->capacity()) - static_cast<difference_type>(rhs.ptr_->
						distance);
				}
			}

			if (rhs.ptr_ == nullptr)
			[[unlikely]]
			{
				return iterator_->distance;
			}

			return static_cast<difference_type>(iterator_->distance) - static_cast<difference_type>(rhs.ptr_->distance);
		}


		// ! need test
		// rely on the implementation of prefix--
		// when n>distance it will continue self-sub from the beginning
		constexpr self& operator-=(const difference_type n)
		{
			if (n == 0)
			[[unlikely]]
			{
				return *this;
			}

			if (n < 0)
			[[unlikely]]
			{
				for (size_t i = 0; i < -n; ++i)
				{
					++(*this);
				}
				return *this;
			}

			// assert (ptr_!=end)
			for (size_t i = 0; i < n; ++i)
			{
				--(*this);
			}

			return *this;
		}

		// ! need test
		// access the circular buffer like a circle
		// when n>capacity it will start over from the beginning again
		constexpr reference operator[](const difference_type n) noexcept
		{
			return *this + n;
		}

		// ! need test
		/**
		 * strong_ordering of ptr_->data
		 */
		constexpr auto operator<=>(const iterator& other) const
		{
			// iterator of different containers cannot be compared => abort
			assert(this->proxy_ == other.proxy_);
			return ptr_->data <=> other.ptr_->data;
		}

		constexpr virtual ~iterator() = default;
	};

	/**
	 * @brief circular iterator
	 *
	 */
	class circular_iterator
	{
		using iterator_concept = std::bidirectional_iterator_tag;
		using iterator_category = std::bidirectional_iterator_tag;
		using value_type = Node;
		using difference_type = ptrdiff_t;
		using pointer = observer_ptr<T>;
		using reference = T&;
		using self = circular_iterator;
		using const_pointer = const observer_ptr<T>;
		using const_reference = const T&;
		using container_ptr = CircularBuffer*;
		using const_container_ptr = const CircularBuffer*;

	private:
		observer_ptr<value_type> ptr_;
		const_container_ptr proxy_;

		auto clone(const iterator& other)
		{
			ptr_ = other.ptr_;
			const_cast<container_ptr&>(proxy_) = other.proxy_;
		}

	protected:
		[[nodiscard]] constexpr auto getProxy_() const
		{
			return proxy_;
		}

	public:
		constexpr circular_iterator() : ptr_(nullptr), proxy_(nullptr)
		{
		}

		constexpr explicit
		circular_iterator(observer_ptr<value_type> ptr, const_container_ptr proxy) : ptr_(ptr), proxy_(proxy)
		{
		}

		constexpr explicit circular_iterator(const circular_iterator& other) : ptr_(other.ptr_), proxy_(other.proxy_)
		{
		}

		constexpr explicit circular_iterator(circular_iterator&& other) noexcept : ptr_(std::move(other.ptr_)),
			proxy_(std::move(other.proxy_))
		{
		}

		constexpr circular_iterator& operator =(const circular_iterator& other)
		{
			if (this == std::addressof(other))
			[[unlikely]]
			{
				return *this;
			}

			this->clone(other);
			return *this;
		}

		constexpr circular_iterator& operator =(circular_iterator&& other) noexcept
		{
			this->clone(other);
			return *this;
		}

		constexpr bool operator==(const circular_iterator& other) const
		{
			// When compare `end` to `end`,
			// if they are the iterators of the some container
			// return true, else return false
			return this->ptr_ == other.ptr_ && this->proxy_ == other.proxy_;
		}

		constexpr reference operator*()
		{
			return this->ptr_->get();
		}

		constexpr const_reference operator*() const
		{
			return this->ptr_->const_get();
		}

		constexpr pointer operator->() const
		{
			return std::addressof(this->ptr_->data);
		}

		/*
		 * @ brief Different from iterator,
		 *		circular_iterator will iterate circular_buffer circularly
		 */
		constexpr self& operator++()
		{
			this->ptr_ = this->ptr_->next;

			return *this;
		}

		/*
		 * @ brief Different from iterator,
		 *		circular_iterator will iterate circular_buffer circularly
		 *	rely on the implementation of prefix++
		 */
		constexpr self operator++(int) &
		{
			self tmp = *this;

			++(*this);

			return tmp;
		}


		/*	! need test
		 * @ brief Different from iterator,
		 *		circular_iterator will iterate circular_buffer circularly
		 */
		constexpr self operator+(const size_t n)
		{
			if (n == 0)
			[[unlikely]]
			{
				return *this;
			}


			auto iterator_ = this->ptr_;


			iterator_ = iterator_->next;


			return circular_iterator(iterator_, this->proxy_);
		}

		// ! need test
		constexpr self& operator+=(const size_t n)
		{
			for (size_t i = 0; i < n; ++i)
			{
				++(*this);
			}

			return *this;
		}

		constexpr self& operator--()
		{
			this->ptr_ = this->ptr_->prev;

			return *this;
		}

		// ! need test
		// rely on the implementation of prefix--
		// when n>distance it will continue self-sub from the beginning
		constexpr self operator--(int) &
		{
			self tmp = *this;
			--(*this);
			return tmp;
		}

		// ! need test
		constexpr self operator-(size_t n) const
		{
			auto iterator_ = this->ptr_;

			for (size_t i = 0; i < n; ++i)
			{
				iterator_ = iterator_->prev;
			}

			return circular_iterator(iterator_, this->proxy_);
		}


		constexpr difference_type operator-(const circular_iterator& rhs) const
		{
			assert(this->proxy_ == rhs.proxy_);

			const auto iterator_ = this->ptr_;

			return iterator_->distance - rhs.ptr_->distance;
		}


		// ! need test
		// rely on the implementation of prefix--
		// when n>distance it will continue self-sub from the beginning
		constexpr self& operator-=(const size_t n)
		{
			for (size_t i = 0; i < n; ++i)
			{
				--(*this);
			}

			return *this;
		}

		// ! need test
		// rely on the implementation of iterator[]
		constexpr reference operator[](const size_t n) noexcept
		{
			auto iterator_ = ptr_;

			for (size_t i = 0; i < n; ++i)
			{
				iterator_ = iterator_->next;
			}

			return *iterator_;
		}

		// ! need test
		constexpr auto operator<=>(const iterator& other) const
		{
			// iterator of different containers cannot be compared => abort
			assert(this->proxy_ == other.proxy_);
			return this->ptr_->data <=> other.ptr_->data;
		}

		constexpr virtual ~circular_iterator() = default;
	};

	/**
	 * @brief reversed iterator
	 *  TODO(Equationzhao)
	 */
	class reverse_iterator
	{
	public:
	};


#pragma endregion

#pragma region Constructor && Descructor

	constexpr explicit CircularBuffer(const size_t capacity_)
	{
		assert(capacity_ != 0); // forbidden
		init(capacity_);
	}

	// TODO(Equationzhao) Support user-defined deleter
	constexpr virtual ~CircularBuffer()
	{
		destroy();
	}

#pragma endregion

#pragma region deleted functions
	/*
			* may implement them later
				or
			? just designed to keep them deleted
	*/
	CircularBuffer(CircularBuffer&& other) = delete;

	CircularBuffer(const CircularBuffer& other) = delete;

	CircularBuffer& operator=(const CircularBuffer& other) = delete;

	CircularBuffer& operator=(CircularBuffer&& other) = delete;

#pragma endregion

#pragma region element access

	[[nodiscard]] constexpr T& front()
	{
		return buffer_->get();
	}

	[[nodiscard]] constexpr T& front() const
	{
		return buffer_->const_get();
	}

	[[nodiscard]] constexpr const T& cfront() const
	{
		return buffer_->const_get();
	}

	[[nodiscard]] constexpr T& back()
	{
		return buffer_->prev->get();
	}

	[[nodiscard]] constexpr T& back() const
	{
		return buffer_->prev->const_get();
	}

	[[nodiscard]] constexpr const T& cback() const
	{
		return buffer_->prev->const_get();
	}

	[[nodiscard]] constexpr T& operator[](const size_t index) noexcept
	{
		auto iterator_ = buffer_;

		for (size_t i = 0; i < index; ++i)
		{
			iterator_ = iterator_->next;
		}

		return iterator_->get();
	}


	// throws runtime_error
	// not recommended
	[[nodiscard]] T& at(const size_t index)
	{
		if (index > size())
		{
			throw std::runtime_error("");
		}

		return this[index];
	}

	[[nodiscard]] auto get_allocator() const
	{
		return alloc_;
	}

	[[nodiscard]] Alloc& get_allocator()
	{
		return alloc_;
	}


#pragma endregion

#pragma region Modifiers
	// TODO(Equationzhao) implementation details
	// Perfect forwarding
	template <typename ...Args>
	constexpr auto write(Args&& ...args)
	{
		toWrite->write(std::forward<Args>(args)...);
		toWrite = toWrite->next;
		// not Implement yet
	}


	template <typename ...Args>
	constexpr auto insert(const iterator where, Args&& ...args)
	{
		this->emplace(where, std::forward<Args>(args)...);
		// not Implement yet
	}

	template <typename ... Args>
	constexpr auto emplace(const iterator where, Args&& ...args)
	{
		assert(where != begin());

		where.ptr_
			? insertNode(where.ptr_, std::forward<Args>(args)...)
			: insertNodeAfter(where.proxy_->buffer_->prev, std::forward<Args>(args)...);


		// not Implement yet
	}

	/*
	 * @brief gets a copy of the data
	 *
	 * @return T
	 */
	constexpr auto read()
	{
		const auto& d = toRead->data;
		toRead = toRead->next;
		return d;
		// not Implement yet
	}

	/*
	 * TODO(Equationzhao) clear the memory
	 */
	constexpr auto clear()
	{
		// not Implement yet
	}

	// constexpr auto merge(raw_ptr<Node> head1,size_t len1, raw_ptr<Node> head2,size_t len2)
	// {
	// 	Node* dummyHead = new Node();
	// 	Node *temp = dummyHead, *temp1 = head1, *temp2 = head2;
	// 	while (temp1 != nullptr && temp2 != nullptr)
	// 	{
	// 		if (temp1->val <= temp2->val)
	// 		{
	// 			temp->next = temp1;
	// 			temp1 = temp1->next;
	// 		}
	// 		else
	// 		{
	// 			temp->next = temp2;
	// 			temp2 = temp2->next;
	// 		}
	// 		temp = temp->next;
	// 	}
	// 	if (temp1 != nullptr)
	// 	{
	// 		temp->next = temp1;
	// 	}
	// 	else if (temp2 != nullptr)
	// 	{
	// 		temp->next = temp2;
	// 	}
	// 	raw_ptr<Node> res = 
	// 	return dummyHead->next;
	// }


	// constexpr auto sort(raw_ptr<Node> begin, raw_ptr<Node> last,
	// 					std::function<bool(const T&, const T&)> fn = std::less<T>())
	// {
	// 	assert(begin <= last);
	//
	//
	//
	// 	if (begin == last)
	// 	{
	// 		return;
	// 	}
	//
	// 	raw_ptr<Node> mid = findNode((begin->distance + last->distance) / 2);
	//
	//
	// 	return merge(sort(begin, mid), ,sort(mid, last),);
	// }

	/*
	 * @brief swaps the pointer and data member
	 *
	 */
	constexpr auto swap(CircularBuffer& rhs) noexcept
	{
		if (std::addressof(rhs) == this)
		{
			return;
		}

		{
			auto temp = this->buffer_;
			this->buffer_ = rhs.buffer_;
			rhs.buffer_ = temp;
		}

		{
			auto temp2 = this->capacity_;
			this->capacity_ = rhs.capacity_;
			rhs.capacity_ = temp2;
		}

		{
			auto temp3 = this->size_;
			this->size_ = rhs.size_;
			rhs.size_ = temp3;
		}

		{
			auto temp4 = this->toWrite;
			this->toWrite = rhs.toWrite;
			rhs.toWrite = temp4;
		}

		{
			auto temp5 = this->toRead;
			this->toRead = rhs.toRead;
			rhs.toRead = temp5;
		}
	}

	/*
	 * @brief reverse the circle buffer
	 * // TODO(Equationzhao) update distance
	 */
	constexpr auto reverse()
	{
		auto iterator_ = buffer_;

		//* exchange the next and prev pointer
		for (size_t i = 0; i < capacity_; ++i)
		{
			auto temp = iterator_->next;
			iterator_->next = iterator_->prev;
			iterator_->prev = temp;
			iterator_ = iterator_->next;
		}
	}

	constexpr auto erase()
	{
		// not Implement yet
	}

	constexpr auto erase_if()
	{
		// not Implement yet
	}


#pragma endregion

#pragma region iterator

	/*
	 *
		Since it's a circle, the begin() and the end() is actually the same element.
		But in order to support range-based-for, the end iterator is designed to be a nullptr
		to access the what the `end` contains will call abort
		operator like prefix++/--,suffix++/--,+=,-= is also **illegal** for the `end` ! 
	 *
	 */


	[[nodiscard]] constexpr iterator begin()
	{
		return iterator(buffer_, this);
	}

	[[nodiscard]] constexpr iterator end()
	{
		return iterator(nullptr, this);
	}

	using const_iterator = const iterator;

	[[nodiscard]] constexpr const_iterator cbegin() const
	{
		return const_iterator(nullptr, this); // TODO
	}

	[[nodiscard]] constexpr const_iterator cend() const
	{
		return const_iterator(nullptr, this); // TODO
	}


	//////////////////////////////////////////////////////////////////////////////////


	[[nodiscard]] circular_iterator circular_begin() const
	{
		return circular_iterator(buffer_, this);
	}

	using const_circular_iterator = circular_iterator;

	[[nodiscard]] const_circular_iterator circular_cbegin() const
	{
		return const_circular_iterator(buffer_, this);
	}


#pragma endregion

#pragma region compare
	[[nodiscard]] constexpr auto operator<=>(const CircularBuffer&) const = default;

	[[nodiscard]] constexpr auto operator==(const CircularBuffer& rhs) const
	{
		if (this->size() != rhs.size())
		{
			return false;
		}

		else
		{
			for (auto i = this->begin(), j = rhs.begin(); i != this->end(); ++i, ++j)
			{
				if (*i != *j)
				{
					return false;
				}
			}

			return true;
		}
	}
#pragma endregion

#pragma region Capacity

	[[nodiscard]] constexpr size_t size() const
	{
		return getSize_();
	}

	[[nodiscard]] constexpr size_t capacity() const
	{
		return getCapacity_();
	}

	[[nodiscard]] constexpr bool empty() const
	{
		return size() == 0;
	}
};

template <std::copyable T, class Alloc>
constexpr typename CircularBuffer<T, Alloc>::iterator::self CircularBuffer<T, Alloc>::iterator::operator+(
	difference_type n) const
{
	if (n == 0)
	[[unlikely]]
	{
		return *this;
	}

	if (n < 0)
	[[unlikely]]
	{
		return *this - (-n);
	}

	assert(ptr_ != nullptr);

	const auto t = n;


	auto iterator_ = ptr_;

	for (size_t i = 0; i < t; ++i)
	{
		if (iterator_ == proxy_->buffer_->prev)
		[[unlikely]]
		{
			iterator_ = nullptr;
		}
		else
		[[likely]]
		{
			iterator_ = iterator_->next;
		}
	}

	return iterator(iterator_, this->proxy_);
}

template <std::copyable T, class Alloc>
constexpr typename CircularBuffer<T, Alloc>::iterator::self CircularBuffer<T, Alloc>::iterator::operator-(
	difference_type n) const
{
	auto iterator_ = ptr_;

	if (n == 0)
	[[unlikely]]
	{
		return *this;
	}

	if (n < 0)
	[[unlikely]]
	{
		return *this + (-n);
	}


	if (iterator_ == nullptr)
	[[unlikely]]
	{
		iterator_ = proxy_->buffer_->prev;
		--n;
	}


	for (difference_type i = 0; i < n; ++i)
	{
		assert(iterator_ == proxy_->buffer_);
		iterator_ = iterator_->prev;
	}

	return iterator(iterator_, this->proxy_);
}

namespace std
{
	template <typename T>
	void swap(CircularBuffer<T>& lhs, CircularBuffer<T>& rhs) noexcept
	{
		lhs.swap(rhs);
	}

	template <typename T>
	void sort(typename CircularBuffer<T>::iterator& begin, typename CircularBuffer<T>::iterator& end,
			  std::function<bool(const typename CircularBuffer<T>::iterator&,
								 const typename CircularBuffer<T>::iterator&)>  =
				  std::less<T>())
	{
	}
} // namespace std


#pragma endregion

#endif // !CIRCULAR_BUFFER


#ifndef CIRCULAR_BUFFER_ADAPTORS
// Fixed capacity
template <std::copyable T, size_t Capacity, class Alloc = std::allocator<T>>
class FixedCircularBuffer
{
	CircularBuffer<T, Alloc> innerContainer_{Capacity};

public:
	constexpr FixedCircularBuffer() = default;
};

// Ownership
template <std::copyable T, size_t Capacity, class Alloc = std::allocator<T>>
class UniqueCircularBuffer
{
	CircularBuffer<T, Alloc> innerContainer_{Capacity};

public:
	constexpr UniqueCircularBuffer() = default;
};

// Thread safe
template <std::copyable T, size_t Capacity, class Alloc = std::allocator<T>>
class SafeCircleBuffer
{
	CircularBuffer<T, Alloc> innerContainer_{Capacity};
public:
	constexpr SafeCircleBuffer() = default;
};
#endif
