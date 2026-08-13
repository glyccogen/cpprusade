#pragma once

#include <cstddef> // for std::nullptr_t
#include <utility> // for std::exchange / std::swap

class SimpleCounter {
    public:
	size_t inc_ref();
	size_t dec_ref();
	size_t ref_count() const;

    private:
	size_t count_ = 0;
};

struct DefaultDelete {
	template <typename T> static void destroy(T * object) {
		delete object;
	}
};

template <typename Derived, typename Counter, typename Deleter> class RefCounted {
    public:
	// Increase reference counter.
	void inc_ref();

	// Decrease reference counter.
	// Destroy object using Deleter when the last instance dies.
	void dec_ref();

	// Get current counter value (the number of strong references).
	size_t ref_count() const;

    private:
	Counter counter_;
};

template <typename Derived, typename D = DefaultDelete>
using SimpleRefCounted = RefCounted<Derived, SimpleCounter, D>;

template <typename T> class IntrusivePtr {
	template <typename Y> friend class IntrusivePtr;

    public:
	// Constructors
	IntrusivePtr();
	IntrusivePtr(std::nullptr_t);
	IntrusivePtr(T * ptr);

	template <typename Y> IntrusivePtr(IntrusivePtr<Y> const & other);

	template <typename Y> IntrusivePtr(IntrusivePtr<Y> && other);

	IntrusivePtr(IntrusivePtr const & other);
	IntrusivePtr(IntrusivePtr && other);

	// `operator=`-s
	IntrusivePtr & operator=(IntrusivePtr const & other);
	IntrusivePtr & operator=(IntrusivePtr && other);

	// Destructor
	~IntrusivePtr();

	// Modifiers
	void reset();
	void reset(T * ptr);
	void swap(IntrusivePtr & other);

	// Observers
	T * get() const;
	T & operator*() const;
	T * operator->() const;
	size_t use_count() const;
	explicit operator bool() const;
};

template <typename T, typename... Args> IntrusivePtr<T> makeIntrusive(Args &&... args);
