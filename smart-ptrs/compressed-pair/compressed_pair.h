#pragma once

#include <type_traits>
#include <utility>

template <typename T, std::size_t Idx, typename Empty = void> class CPElement {
    public:
	CPElement(): val_{} {};
	template <typename TT> CPElement(TT && v) : val_{std::forward<TT>(v)} {};
	T const & get() const {
		return val_;
	}

	T & get() {
		return val_;
	}

    private:
	T val_;
};

template <typename T, std::size_t Idx>
class CPElement<T, Idx, std::enable_if_t<std::is_empty_v<T> && !std::is_final_v<T>>> : T {
    public:
	CPElement(): T{} {};
	template <typename TT> CPElement(TT && v) : T{std::forward<TT>(v)} {};
	T const & get() const {
		return static_cast<T const &>(*this);
	}

	T & get() {
		return static_cast<T &>(*this);
	}
};

// Me think, why waste time write lot code, when few code do trick.
template <typename F, typename S> class CompressedPair : CPElement<F, 0>, CPElement<S, 1> {
	using FB = CPElement<F, 0>;
	using SB = CPElement<S, 1>;

    public:
	CompressedPair(): FB{}, SB{} {}
	template <typename FF, typename SS>
	CompressedPair(FF && f, SS && s) : FB{std::forward<FF>(f)}, SB{std::forward<SS>(s)} {}

	F & get_first() {
		return FB::get();
	}

	F const & get_first() const {
		return static_cast<FB const &>(*this).get();
	}

	S & get_second() {
		return SB::get();
	}

	S const & get_second() const {
		return static_cast<SB const &>(*this).get();
	}
};
