/////////////////////////
//                     //
//     EXPERIMENTAL    //
//    NOT UP TO DATE   //
//                     //
/////////////////////////










export module Circular_Buffer;


#pragma region Includes
import <cassert>;
import <concepts>;
import <cstddef>;
import <functional>;
import <iterator>;
import <new>;
import <stdexcept>;
import <utility>;

#pragma endregion

template <typename T>
using observer_ptr = T*;

template <typename T>
constexpr auto make_observer(T& obj)
{
	return std::addressof(obj);
}


// template<typename T, class Allocator = std::allocator<T>>
export template <std::copyable T, class Alloc = std::allocator<T>>
/*
	TODO(Equationzhao):
		* refactor size_t in operator +,-,+=,-= of iterator
		* Add concept Support
		* Add constexpr support
		* Add user-defined comparator support
		* Add user-defined deleter support
		* Use placement new instead of new expression
		? Module support
		? allocator support
		? ranges support
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
	public:
		using value_type = T;
#pragma region data and pointer
		value_type data{};
		observer_ptr<Node> next{ nullptr };
		observer_ptr<Node> prev{ nullptr };
		observer_ptr<Node> head{ nullptr };
		size_t distance{ 0 };
		//TODO(Equationzhao) refactor :use distance,
		// ? is the variable head still necessary ?
#pragma endregion

#pragma region Constructors && Destructor

		constexpr explicit Node(const T& data) : data(data)
		{
		}

		constexpr explicit Node(T&& data) : data(std::move(data))
		{
		}

		constexpr Node() = default;

		constexpr Node(const Node& other) : data(other.data), next(other.next), prev(other.prev),
			distance(other.distance)
		{
		}

		constexpr Node(Node&& other) noexcept : data(std::move(other.data)), next(other.next), prev(other.prev),
			distance(other.distance)
		{
		}

		constexpr Node& operator=(const Node& other)
		{
			data = (other.data);
			next = (other.next);
			prev = (other.prev);
			distance = (other.distance);
			return *this;
		}

		constexpr Node& operator=(Node&& other) noexcept
		{
			data = std::move(other.data);
			next = (other.next);
			prev = (other.prev);
			distance = (other.distance);
			return *this;
		}

		constexpr virtual ~Node() = default;

		// ? compare the T data it contains
		//		or
		//	 test `are they actually the same object`
		constexpr bool operator==(const Node& rhs) const
		{
			return this->data == rhs.data;
		}

		constexpr auto operator<=>(const Node& rhs) const
		{
			return this->data <=> rhs.data;
		}
#pragma endregion

#pragma region Modifiers
		/*
		 * @brief write data
		 */
		auto write(const T& dataToWrite)
		{
			this->data = dataToWrite;
		}

		/*
		 * @brief write data
		 */
		auto write(T&& dataToWrite)
		{
			this->data = std::move(dataToWrite);
		}

		/*
		 * @brief get a copy of the data
		 * @return value
		 */
		[[nodiscard]] T read() const
		{
			return this->data;
		}


		/**
		 * @brief  get Reference of the data
		 * @return reference
		 */
		[[nodiscard]] T& get()
		{
			return this->data;
		}

		/**
		 * @brief  get Reference of the data
		 * @return reference
		 */
		[[nodiscard]] T& get() const
		{
			return this->data;
		}


		/**
		 * @brief  get const-reference of the data
		 * @return  const reference
		 */
		[[nodiscard]] const T& const_get() const
		{
			return this->data;
		}


#pragma endregion
	};

	observer_ptr<Node> buffer;
	size_t capacity_{ 0 };
	size_t size_{ 0 };

	observer_ptr<Node> toWrite;
	observer_ptr<Node> toRead;

	/**
	 * @brief initialize the circular buffer
	 *
	 * @param capacityToInit the capacity of the buffer
	 */
	constexpr void init(size_t capacityToInit)
	{
		this->capacity_ = capacityToInit;
		this->size_ = 0;


#pragma region initialize buffer
		// create buffer
		buffer = new Node();
		observer_ptr<Node> iterator_ = buffer;

		iterator_->head = buffer;


		// create nodes and link them
		// update head and distance
		size_t distance{ 0 };
		iterator_->distance = distance;

		for (size_t i = 0; i < capacityToInit - 1; ++i)
		{
			iterator_->next = new Node();
			iterator_->next->prev = iterator_;
			iterator_ = iterator_->next;
			iterator_->distance = ++distance;
			iterator_->head = buffer;
		}

		// make it circular
		iterator_->next = buffer;
		buffer->prev = iterator_;

		toWrite = buffer;
		toRead = buffer;

#pragma endregion
	}

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

		if (buffer == nullptr)
		{
			return;
		}

		auto iterator_ = buffer;

		//* Recursively delete
		//! need Test!
		//BUG: StackOverFlow
		const auto deleterBack = [this](observer_ptr<Node> node)
		{
			auto head_ = this;
			static std::function<void(observer_ptr<Node>)> deleter_;
			deleter_ = [&head_](observer_ptr<Node> node)
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
		return buffer->get();
	}

	[[nodiscard]] auto getTailElement()
	{
		return buffer->prev->get();
	}

	[[nodiscard]] auto getHeadElement() const
	{
		return buffer->const_get();
	}

	[[nodiscard]] auto getTailElement() const
	{
		return buffer->prev->const_get();
	}

	[[nodiscard]] auto getPtrHead_()
	{
		return std::addressof(buffer->data);
	}

	[[nodiscard]] auto getPtrTail_()
	{
		return std::addressof(buffer->prev->data);
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
		using const_pointer = const observer_ptr<value_type>;
		using const_reference = const value_type&;
		using container_ptr = CircularBuffer*;
		using const_container_ptr = const CircularBuffer*;
	private:
		observer_ptr<value_type> ptr_;
		const_container_ptr proxy_;

		auto clone(const iterator& other)
		{
			ptr_ = other.ptr_;
			proxy_ = other.proxy_;
		}

	protected:
		[[nodiscard]] auto getProxy_() const
		{
			return proxy_;
		}


	public:
		constexpr inline static observer_ptr<Node> end{ nullptr };


		iterator() : ptr_(nullptr), proxy_(nullptr)
		{
		}

		explicit iterator(observer_ptr<value_type> ptr, const_container_ptr proxy) : ptr_(ptr), proxy_(proxy)
		{
		}

		iterator(const iterator& other) : ptr_(other.ptr_), proxy_(other.proxy_)
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
			assert(this->ptr_ != end);

			return ptr_->get();
		}

		constexpr const_reference operator*() const
		{
			assert(this->ptr_ != end);

			return ptr_->const_get();
		}

		constexpr pointer operator->() const
		{
			assert(this->ptr_ != end);

			return std::addressof(ptr_->data);
		}

		constexpr self& operator++()
		{
			assert(ptr_ != end);

			if (ptr_ == ptr_->head->prev)
				[[unlikely]]
			{
				ptr_ = end;
			}
			else
				[[likely]]
			{
				ptr_ = ptr_->next;
			}

			return *this;
		}


		constexpr self operator++(int)&
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
			// assert(ptr_ != end);

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

			for (size_t i = 0; i < n; ++i)
			{
				++(*this);
			}

			return *this;
		}

		// When it's begin(), -- will call abort
		constexpr self& operator--()
		{
			assert(this->ptr_ == proxy_->buffer);

			if (this->ptr_ == end)
				[[unlikely]]
			{
				this->ptr_ = proxy_->buffer->prev;
			}


			ptr_ = ptr_->prev;

			return *this;
		}

		// ! need test
		// rely on the implementation of prefix--
		// when n>distance it will continue self-sub from the beginning
		constexpr self operator--(int)&
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

			if (iterator_ == end)
				[[unlikely]]
			{
				if (rhs.ptr_ == end)
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

				if (rhs.ptr_ == end)
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
		using const_pointer = const observer_ptr<value_type>;
		using const_reference = const value_type&;
		using container_ptr = CircularBuffer*;
		using const_container_ptr = const CircularBuffer*;

	private:
		observer_ptr<value_type> ptr_;
		const_container_ptr proxy_;

		constexpr auto clone(const iterator& other)
		{
			ptr_ = other.ptr_;
			proxy_ = other.proxy_;
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
		constexpr self operator++(int)&
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
		constexpr self operator--(int)&
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
#pragma endregion

#pragma region Constructor && Descructor

	constexpr explicit CircularBuffer(size_t capacity_)
	{
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

	explicit CircularBuffer(const CircularBuffer& other) = delete;

	CircularBuffer& operator=(const CircularBuffer& other) = delete;

	CircularBuffer& operator=(CircularBuffer&& other) = delete;

#pragma endregion

#pragma region element access

	[[nodiscard]] constexpr T& front()
	{
		return buffer->get();
	}

	[[nodiscard]] constexpr T& front() const
	{
		return buffer->const_get();
	}

	[[nodiscard]] constexpr const T& cfront() const
	{
		return buffer->const_get();
	}

	[[nodiscard]] constexpr T& back()
	{
		return buffer->prev->get();
	}

	[[nodiscard]] constexpr T& back() const
	{
		return buffer->prev->const_get();
	}

	[[nodiscard]] constexpr const T& cback() const
	{
		return buffer->prev->const_get();
	}

	[[nodiscard]] constexpr T& operator[](const size_t index) noexcept
	{
		auto iterator_ = buffer;

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
#pragma endregion

#pragma region Modifiers
	// TODO(Equationzhao) implementation details
	// Perfect forwarding
	constexpr auto write(T&& data)
	{
		toWrite->write(std::move(data));
		toWrite = toWrite->next;
		// not Implement yet
	}


	constexpr auto write(const T& data)
	{
		toWrite->write(data);
		toWrite = toWrite->next;
	}

	constexpr auto insert()
	{
		// not Implement yet
	}

	constexpr auto emplace()
	{
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

	constexpr auto sort()
	{
		// not Implement yet
	}

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
			auto temp = this->buffer;
			this->buffer = rhs.buffer;
			rhs.buffer = temp;
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
		auto iterator_ = buffer;

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
		operator like prefix++/--,suffix++/--,+=,-= is legal for the `end`
	 *
	 */


	[[nodiscard]] constexpr iterator begin()
	{
		return iterator(buffer, this);
	}

	[[nodiscard]] constexpr iterator end()
	{
		return iterator(iterator::end, this);
	}

	using const_iterator = const iterator;

	[[nodiscard]] constexpr const_iterator cbegin() const
	{
		return const_iterator(iterator::end, this);
	}

	[[nodiscard]] constexpr const_iterator cend() const
	{
		return const_iterator(iterator::end, this);
	}


	//////////////////////////////////////////////////////////////////////////////////


	[[nodiscard]] circular_iterator circular_begin() const
	{
		return circular_iterator(buffer, this);
	}

	using const_circular_iterator = circular_iterator;

	[[nodiscard]] const_circular_iterator circular_cbegin() const
	{
		return const_circular_iterator(buffer, this);
	}


#pragma endregion

#pragma region compare
	[[nodiscard]] constexpr auto operator<=>(const CircularBuffer&) const = default;

	[[nodiscard]] constexpr auto operator==(const CircularBuffer & rhs) const
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

	assert(ptr_ != end);

	const auto t = n;


	auto iterator_ = ptr_;

	for (size_t i = 0; i < t; ++i)
	{
		if (iterator_ == iterator_->head->prev)
			[[unlikely]]
		{
			iterator_ = end;
		}
		else
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


		if (iterator_ == end)
			[[unlikely]]
	{
		iterator_ = proxy_->buffer->prev;
		--n;
	}


	for (difference_type i = 0; i < n; ++i)
	{
		assert(iterator_ == proxy_->buffer);
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
}


#pragma endregion



#pragma region CIRCULAR_BUFFER_ADAPTORS
// Fixed capacity
template <std::copyable T, size_t Capacity, class Alloc = std::allocator<T>>
class FixedCircularBuffer
{
	CircularBuffer<T, Alloc> innerContainer_{ Capacity };

public:
	constexpr FixedCircularBuffer() = default;
};

// Ownership
template <std::copyable T, size_t Capacity, class Alloc = std::allocator<T>>
class UniqueCircularBuffer
{
	CircularBuffer<T, Alloc> innerContainer_{ Capacity };

public:
	constexpr UniqueCircularBuffer() = default;
};

// Thread safe
template <std::copyable T, size_t Capacity, class Alloc = std::allocator<T>>
class SafeCircleBuffer
{
	CircularBuffer<T, Alloc> innerContainer_{ Capacity };
public:
	constexpr SafeCircleBuffer() = default;
};
#pragma endregion
