#pragma once

#include <algorithm>
#include <cstddef>
#include <type_traits>
#include <utility>
// Me think, why waste time write lot code, when few code do trick.
template <typename F, typename S> class CompressedPair {
    public:
	CompressedPair() : buf_{} {}
	template <typename FF, typename SS> CompressedPair(FF && f, SS && s) : buf_{} {
		new (buf_) F{std::forward<FF>(f)};
		new (buf_ + kFirstSz) S{std::forward<SS>(s)};
	}

	F & get_first() {
		return *reinterpret_cast<F *>(buf_);
	}

	F const & get_first() const {
		return *reinterpret_cast<F const *>(buf_);
	}

	S & get_second() {
		return *reinterpret_cast<S *>(buf_ + kFirstSz);
	}
	S const & get_second() const {
		return *reinterpret_cast<S const *>(buf_ + kFirstSz);
	};

	~CompressedPair() {
		reinterpret_cast<F *>(buf_)->~F();
		reinterpret_cast<S *>(buf_ + kFirstSz)->~S();
	}

    private:
	static constexpr size_t kFirstSz = std::is_empty_v<F> ? 0 : sizeof(F);
	static constexpr size_t kSecondSz = std::is_empty_v<S> ? 0 : sizeof(S);
	static constexpr size_t kSz = kFirstSz + kSecondSz == 0 ? 1 : kFirstSz + kSecondSz;
	alignas(std::max(alignof(F), alignof(S))) std::byte buf_[kSz];
};
