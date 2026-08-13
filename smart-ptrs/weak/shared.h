#pragma once

#include "sw_fwd.h" // Forward declaration

#include <cstddef> // std::nullptr_t

// https://en.cppreference.com/w/cpp/memory/shared_ptr
template <typename T> class SharedPtr {
    public:
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Constructors

	SharedPtr();
	SharedPtr(std::nullptr_t);
	explicit SharedPtr(T * ptr);

	SharedPtr(SharedPtr const & other);
	SharedPtr(SharedPtr && other);

	// Aliasing constructor
	// #8 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
	template <typename Y> SharedPtr(SharedPtr<Y> const & other, T * ptr);

	// Promote `WeakPtr`
	// #11 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
	explicit SharedPtr(WeakPtr<T> const & other);

	////////////////////////////////////////////////////////////////////////////////////////////////
	// `operator=`-s

	SharedPtr & operator=(SharedPtr const & other);
	SharedPtr & operator=(SharedPtr && other);

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Destructor

	~SharedPtr();

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Modifiers

	void reset();
	void reset(T * ptr);
	void swap(SharedPtr & other);

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Observers

	T * get() const;
	T & operator*() const;
	T * operator->() const;
	size_t use_count() const;
	explicit operator bool() const;
};

template <typename T, typename U>
inline bool operator==(SharedPtr<T> const & left, SharedPtr<U> const & right);

// Allocate memory only once
template <typename T, typename... Args> SharedPtr<T> makeShared(Args &&... args);

// Look for usage examples in tests
template <typename T> class EnableSharedFromThis {
    public:
	SharedPtr<T> shared_from_this();
	SharedPtr<T const> shared_from_this() const;

	WeakPtr<T> weak_from_this() noexcept;
	WeakPtr<T const> weak_from_this() const noexcept;
};
