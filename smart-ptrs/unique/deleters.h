#pragma once

#include <cassert>
#include <type_traits>

template <class T> class Deleter {
    public:
	Deleter() = default;

	Deleter(int tag) : tag_(tag) {}

	Deleter(Deleter const &) = delete;

	Deleter(Deleter && rhs) noexcept : tag_(rhs.tag_) {
		rhs.tag_ = 0;
	}

	Deleter & operator=(Deleter const &) = delete;

	Deleter & operator=(Deleter && r) noexcept {
		tag_ = r.tag_;
		r.tag_ = 0;
		return *this;
	}

	~Deleter() = default;

	int get_tag() const {
		return tag_;
	}

	void operator()(T * p) const {
		static_assert(sizeof(T) > 0);
		static_assert(!std::is_void_v<T>);
		delete p;
		was_called_ = true;
	}

	bool is_const() const {
		return true;
	}

	bool is_const() {
		return false;
	}

	bool was_called() const {
		return was_called_;
	}

    private:
	int tag_ = 0;
	mutable bool was_called_ = false;
};

template <class T> class Deleter<T[]> {
    public:
	Deleter() = default;

	Deleter(int tag) : tag_(tag) {}

	Deleter(Deleter const &) = delete;

	Deleter(Deleter && rhs) noexcept : tag_(rhs.tag_) {
		rhs.tag_ = 0;
	}

	Deleter & operator=(Deleter const &) = delete;

	Deleter & operator=(Deleter && r) noexcept {
		tag_ = r.tag_;
		r.tag_ = 0;
		return *this;
	}

	~Deleter() = default;

	int get_tag() const {
		return tag_;
	}

	void operator()(T * p) const {
		static_assert(sizeof(T) > 0);
		static_assert(!std::is_void_v<T>);
		delete[] p;
	}

	bool is_const() const {
		return true;
	}

	bool is_const() {
		return false;
	}

    private:
	int tag_ = 0;
};

template <typename T> class CopyableDeleter {
    public:
	CopyableDeleter() = default;

	CopyableDeleter(int tag) : tag_(tag) {}

	CopyableDeleter(CopyableDeleter const &) = default;

	CopyableDeleter(CopyableDeleter && rhs) noexcept : tag_(rhs.tag_) {
		rhs.tag_ = 0;
	}

	CopyableDeleter & operator=(CopyableDeleter const &) = default;

	CopyableDeleter & operator=(CopyableDeleter && r) noexcept {
		tag_ = r.tag_;
		r.tag_ = 0;
		return *this;
	}

	~CopyableDeleter() = default;

	int get_tag() const {
		return tag_;
	}

	void operator()(T * p) const {
		static_assert(sizeof(T) > 0);
		static_assert(!std::is_void_v<T>);
		delete p;
	}

	bool is_const() const {
		return true;
	}

	bool is_const() {
		return false;
	}

    private:
	int tag_ = 0;
};
