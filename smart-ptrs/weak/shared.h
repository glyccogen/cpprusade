#pragma once

#include "sw_fwd.h" // Forward declaration

#include <cstddef> // std::nullptr_t
#include <type_traits>
#include <utility>

struct ControlBlockBase {
	ControlBlockBase() : strong_{1}, weak_{1} {};
	size_t strong_;
	size_t weak_;
	virtual ~ControlBlockBase() = default;
	virtual void destroy_obj() = 0;
};

template <typename U> struct ControlBlockWithObj : ControlBlockBase {
	alignas(U) std::byte buf[sizeof(U)];

	U * ptr() {
		return reinterpret_cast<U *>(buf);
	}

	template <typename... Args> ControlBlockWithObj(Args &&... args) {
		new (buf) U(std::forward<Args>(args)...);
	}

	~ControlBlockWithObj() override = default;
	void destroy_obj() override {
		ptr()->~U();
	}
};

template <typename U> struct ControlBlockWithPtr : ControlBlockBase {
	U * ptr_;
	ControlBlockWithPtr(U * ptr) : ptr_{ptr} {};

	~ControlBlockWithPtr() override = default;

	void destroy_obj() override {
		delete ptr_;
		ptr_ = nullptr;
	};
};

// https://en.cppreference.com/w/cpp/memory/shared_ptr
template <typename T> class SharedPtr {
	ControlBlockBase * cb_;
	T * ptr_;
	void inc_strong() {
		if(cb_) {
			++cb_->strong_;
			if(cb_->strong_ == 1) {
				++cb_->weak_;
			}
		}
	}
	template <typename V> void copy_from(SharedPtr<V> const & other) {
		reset();
		cb_ = other.cb_;
		ptr_ = other.ptr_;
		inc_strong();
	}

	template <typename V> void move_from(SharedPtr<V> && other) {
		reset();
		cb_ = other.cb_;
		ptr_ = other.ptr_;
		other.cb_ = nullptr;
		other.ptr_ = nullptr;
	}

	template <typename U> friend class SharedPtr;

	template <typename U> friend class WeakPtr;

	template <typename U, typename... Args> friend SharedPtr<U> makeShared(Args &&... args);
	// FIXME

	SharedPtr(ControlBlockBase * cb, T * ptr) : cb_{cb}, ptr_{ptr} {
		if(!cb_ || !cb_->strong_) {
			ptr_ = nullptr;
		}
	}

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
		inc_strong();
	}
	SharedPtr(SharedPtr && other) : cb_{other.cb_}, ptr_{other.get()} {
		other.cb_ = nullptr;
		other.ptr_ = nullptr;
	}

	template <typename V>
	SharedPtr(SharedPtr<V> const & other)
	        requires(std::is_convertible_v<V *, T *>)
	        : cb_{other.cb_}, ptr_{other.ptr_} {
		inc_strong();
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
		inc_strong();
	}
	// Promote `WeakPtr`
	// #11 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
	explicit SharedPtr(WeakPtr<T> const & other) : SharedPtr() {
		cb_ = other.cb_;
		ptr_ = other.ptr_;
		if(!cb_ || !cb_->strong_) {
			throw BadWeakPtr();
		}
		inc_strong();
	};

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
		if(cb_) {
			if(--cb_->strong_ == 0) {
				cb_->destroy_obj();
				if(--cb_->weak_ == 0) {
					delete cb_;
				}
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
		return cb_->strong_;
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
	auto ptr = cb->ptr();
	return SharedPtr<U>{cb, ptr};
}

// Look for usage examples in tests
// FIXME
template <typename T> class EnableSharedFromThis {
    public:
	SharedPtr<T> shared_from_this();
	SharedPtr<T const> shared_from_this() const;

	WeakPtr<T> weak_from_this() noexcept;
	WeakPtr<T const> weak_from_this() const noexcept;
};
