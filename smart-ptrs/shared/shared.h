#pragma once

#include "sw_fwd.h" // Forward declaration

#include <cstddef> // std::nullptr_t
#include <type_traits>
#include <utility>

struct ControlBlockBase {
	ControlBlockBase() : cnt_{1} {};
	size_t cnt_;
	virtual ~ControlBlockBase() = default;
};

template <typename U> struct ControlBlockWithObj : ControlBlockBase {
	U data_;
	template <typename... Args>
	ControlBlockWithObj(Args &&... args) : data_{std::forward<Args>(args)...} {}

	~ControlBlockWithObj() override = default;
};

template <typename U> struct ControlBlockWithPtr : ControlBlockBase {
	U * ptr_;
	ControlBlockWithPtr(U * ptr) : ptr_{ptr} {};

	~ControlBlockWithPtr() override {
		delete ptr_;
	}
};

// https://en.cppreference.com/w/cpp/memory/shared_ptr
template <typename T> class SharedPtr {
	ControlBlockBase * cb_;
	T * ptr_;
	void inc_cnt() {
		if(cb_) {
			++cb_->cnt_;
		}
	}
	template <typename V> void copy_from(SharedPtr<V> const & other) {
		reset();
		cb_ = other.cb_;
		ptr_ = other.ptr_;
		inc_cnt();
	}

	template <typename V> void move_from(SharedPtr<V> && other) {
		reset();
		cb_ = other.cb_;
		ptr_ = other.ptr_;
		other.cb_ = nullptr;
		other.ptr_ = nullptr;
	}

	template <typename U> friend class SharedPtr;

	template <typename U, typename... Args> friend SharedPtr<U> makeShared(Args &&... args);

	SharedPtr(ControlBlockBase * cb, T * ptr) : cb_{cb}, ptr_{ptr} {}

    public:
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Constructors

	SharedPtr() : cb_{nullptr}, ptr_{nullptr} {}
	SharedPtr(std::nullptr_t) : SharedPtr() {}
	template <typename U>
	explicit SharedPtr(U * ptr)
	        requires(std::is_convertible_v<U *, T *>)
	        : cb_{new ControlBlockWithPtr<U>{ptr}}, ptr_{ptr} {}

	SharedPtr(SharedPtr const & other) : cb_{other.cb_}, ptr_{other.get()} {
		inc_cnt();
	}
	SharedPtr(SharedPtr && other) : cb_{other.cb_}, ptr_{other.get()} {
		other.cb_ = nullptr;
		other.ptr_ = nullptr;
	}

	template <typename V>
	SharedPtr(SharedPtr<V> const & other)
	        requires(std::is_convertible_v<V *, T *>)
	        : cb_{other.cb_}, ptr_{other.ptr_} {
		inc_cnt();
	}

	template <typename V>
	SharedPtr(SharedPtr<V> && other)
	        requires(std::is_convertible_v<V *, T *>)
	        : cb_{other.cb_}, ptr_{other.ptr_} {
		other.cb_ = nullptr;
		other.ptr_ = nullptr;
	}

	// Aliasing constructor
	// #8 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
	template <typename Y>
	SharedPtr(SharedPtr<Y> const & other, T * ptr) : cb_{other.cb_}, ptr_{ptr} {
		inc_cnt();
	}
	// Promote `WeakPtr`
	// #11 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
	explicit SharedPtr(WeakPtr<T> const & other);

	////////////////////////////////////////////////////////////////////////////////////////////////
	// `operator=`-s

	SharedPtr & operator=(SharedPtr const & other) {
		if(this != &other) {
			copy_from(other);
		}
		return *this;
	}
	SharedPtr & operator=(SharedPtr && other) {
		if(this != &other) {
			move_from(std::move(other));
		}
		return *this;
	}

	template <typename V>
	SharedPtr & operator=(SharedPtr<V> const & other)
	        requires(std::is_convertible_v<V *, T *>)
	{
		copy_from(other);

		return *this;
	}

	template <typename V>
	SharedPtr & operator=(SharedPtr<V> && other)
	        requires(std::is_convertible_v<V *, T *>)
	{
		move_from(std::move(other));
		return *this;
	}
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Destructor

	~SharedPtr() {
		reset();
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Modifiers

	void reset() {
		if(this->cb_) {
			--cb_->cnt_;
			if(cb_->cnt_ == 0) {
				delete cb_;
			}
		}
		cb_ = nullptr;
		ptr_ = nullptr;
	}
	template <typename U>
	void reset(U * ptr)
	        requires(std::is_convertible_v<U *, T *>)
	{
		reset();
		if(ptr) {
			cb_ = new ControlBlockWithPtr{ptr};
			ptr_ = ptr;
		}
	}
	void swap(SharedPtr & other) {
		std::swap(cb_, other.cb_);
		std::swap(ptr_, other.ptr_);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Observers

	T * get() const {
		return ptr_;
	}
	T & operator*() const {
		return *ptr_;
	}
	T * operator->() const {
		return ptr_;
	}
	size_t use_count() const {
		if(!cb_) {
			return 0;
		}
		return cb_->cnt_;
	}
	explicit operator bool() const {
		return ptr_;
	}
};

template <typename T, typename U>
inline bool operator==(SharedPtr<T> const & left, SharedPtr<U> const & right) {
	return left.get() == right.get();
}

// Allocate memory only once
template <typename U, typename... Args> SharedPtr<U> makeShared(Args &&... args) {
	auto * cb = new ControlBlockWithObj<U>{std::forward<Args>(args)...};
	auto ptr = &cb->data_;
	return SharedPtr<U>{cb, ptr};
}

// Look for usage examples in tests
template <typename T> class EnableSharedFromThis {
    public:
	SharedPtr<T> shared_from_this();
	SharedPtr<T const> shared_from_this() const;

	WeakPtr<T> weak_from_this() noexcept;
	WeakPtr<T const> weak_from_this() const noexcept;
};
