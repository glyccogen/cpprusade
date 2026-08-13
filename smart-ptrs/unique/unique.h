#pragma once

#include "compressed_pair.h"

#include <cstddef> // std::nullptr_t
#include <type_traits>

template <typename U> struct DefaultDelete {
	DefaultDelete() = default;
	void operator()(U * ptr) const {
		delete ptr;
	}

	template <typename V>
	DefaultDelete(DefaultDelete<V> &&)
	        requires(std::is_convertible_v<V*, U*>)
	{}
};

template <typename U> struct DefaultDelete<U[]> {
	DefaultDelete() = default;
	void operator()(U * ptr) const {
		delete[] ptr;
	}
	template <typename V>
	DefaultDelete(DefaultDelete<V> &&)
	        requires(std::is_convertible_v<V*, U*>)
	{}
};

// Primary template
template <typename U, typename Deleter = DefaultDelete<U>> class UniquePtr {
    public:
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Constructors

	explicit UniquePtr(U * ptr = nullptr) : pair_{ptr, Deleter{}} {};
	UniquePtr(U * ptr, Deleter deleter) : pair_{ptr, std::move(deleter)} {};

	template <typename UU, typename DD>
	UniquePtr(UniquePtr<UU, DD> && other) noexcept
	        requires(std::is_convertible_v<UU *, U *> && std::is_convertible_v<DD, Deleter>)
	        : UniquePtr{other.release(), std::move(other.get_deleter())} {}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// `operator=`-s

	UniquePtr & operator=(UniquePtr && other) noexcept {
		if(this == &other) {
			return *this;
		}
		swap(other);
		other.reset();
		return *this;
	}
	UniquePtr & operator=(std::nullptr_t) {
		reset();
		return *this;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Destructor

	~UniquePtr() {
		reset();
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Modifiers

	U * release() {
		auto res = get();
		pair_.get_first() = nullptr;
		return res;
	}
	void reset(U * ptr = nullptr) {
		auto old = get();
		pair_.get_first() = ptr;
		if(old) {
			get_deleter()(old);
		}
	}
	void swap(UniquePtr & other) {
		std::swap(pair_, other.pair_);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Observers

	U * get() const {
		return pair_.get_first();
	}
	Deleter & get_deleter() {
		return pair_.get_second();
	}

	Deleter const & get_deleter() const {
		return pair_.get_second();
	}
	explicit operator bool() const {
		return get() != nullptr;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Single-object dereference operators

	auto & operator*() const
	        requires(!std::is_void_v<U> && !std::is_array_v<std::remove_cvref_t<U>>)
	{
		return *get();
	}

	U * operator->() const
	        requires(!std::is_void_v<U>)
	{
		return get();
	}

    private:
	CompressedPair<U *, Deleter> pair_;
};

// Specialization for arrays
template <typename U, typename Deleter> class UniquePtr<U[], Deleter> {
    public:
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Constructors

	explicit UniquePtr(U * ptr = nullptr) : pair_{ptr, Deleter{}} {};
	UniquePtr(U * ptr, Deleter deleter) : pair_{ptr, std::move(deleter)} {};

	template <typename UU, typename DD>
	UniquePtr(UniquePtr<UU, DD> && other) noexcept
	        requires(std::is_convertible_v<UU *, U *> && std::is_convertible_v<DD, Deleter>)
	        : UniquePtr{other.release(), std::move(other.get_deleter())} {}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// `operator=`-s

	UniquePtr & operator=(UniquePtr && other) noexcept {
		if(this == &other) {
			return *this;
		}
		swap(other);
		other.reset();
		return *this;
	}
	UniquePtr & operator=(std::nullptr_t) {
		reset();
		return *this;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Destructor

	~UniquePtr() {
		reset();
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Modifiers

	U * release() {
		auto res = get();
		pair_.get_first() = nullptr;
		return res;
	}
	void reset(U * ptr = nullptr) {
		auto old = get();
		pair_.get_first() = ptr;
		if(old) {
			get_deleter()(old);
		}
	}
	void swap(UniquePtr & other) {
		std::swap(pair_, other.pair_);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Observers

	U * get() const {
		return pair_.get_first();
	}
	Deleter & get_deleter() {
		return pair_.get_second();
	}

	Deleter const & get_deleter() const {
		return pair_.get_second();
	}
	explicit operator bool() const {
		return get() != nullptr;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Single-object dereference operators

	U & operator[](size_t idx) {
		return get()[idx];
	}

    private:
	CompressedPair<U *, Deleter> pair_;
};
