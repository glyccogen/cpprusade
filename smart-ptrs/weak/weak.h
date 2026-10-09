#pragma once

#include "sw_fwd.h" // Forward declaration
#include "shared.h"
#include <utility>

// https://en.cppreference.com/w/cpp/memory/weak_ptr
template <typename T> class WeakPtr {

	ControlBlockBase * cb_;
	T * ptr_;

	void inc_weak() {
		if(cb_) {
			++cb_->weak_;
		}
	}

	void copy_from(WeakPtr const & other) {
		reset();
		cb_ = other.cb_;
		ptr_ = other.ptr_;
		inc_weak();
	}

	void move_from(WeakPtr && other) {
		reset();
		cb_ = other.cb_;
		ptr_ = other.ptr_;
		other.cb_ = nullptr;
		other.ptr_ = nullptr;
	}

	template <typename U> friend class SharedPtr;

    public:
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Constructors

	WeakPtr() : cb_{nullptr}, ptr_{nullptr} {}

	WeakPtr(WeakPtr const & other) : cb_{other.cb_}, ptr_{other.ptr_} {
		inc_weak();
	}

	WeakPtr(WeakPtr && other) : WeakPtr(other) {
		other.cb_ = nullptr;
		other.ptr_ = nullptr;
	}

	// Demote `SharedPtr`
	// #2 from https://en.cppreference.com/w/cpp/memory/weak_ptr/weak_ptr
	WeakPtr(SharedPtr<T> const & other) {
		cb_ = other.cb_;
		ptr_ = other.ptr_;
		inc_weak();
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// `operator=`-s

	WeakPtr & operator=(WeakPtr const & other) {
		if(this != &other) {
			copy_from(other);
		}
		return *this;
	}
	WeakPtr & operator=(WeakPtr && other) {
		if(this != &other) {
			move_from(std::move(other));
		}
		return *this;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Destructor

	~WeakPtr() {
		reset();
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Modifiers

	void reset() {
		if(this->cb_) {
			if(--cb_->weak_ == 0) {
				delete cb_;
			}
		}
		cb_ = nullptr;
		ptr_ = nullptr;
	}

	void swap(WeakPtr & other) {
		std::swap(cb_, other.cb_);
		std::swap(ptr_, other.ptr_);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Observers

	size_t use_count() const {
		return cb_ ? cb_->strong_ : 0;
	}
	bool expired() const {
		return !use_count();
	}
	SharedPtr<T> lock() const {
		auto res = SharedPtr<T>{cb_, ptr_};
		res.inc_strong();
		return res;
	}
};
