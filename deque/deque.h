#pragma once

#include <cstddef>
#include <initializer_list>
#include <algorithm>
#include <utility>

class Deque {
    public:
	Deque() : blocks_{nullptr}, blocks_sz_{0}, left_{0}, sz_{0} {}
	Deque(Deque const & rhs)
	        : blocks_{rhs.sz_ == 0 ? nullptr
	                               : new int *[(rhs.sz_ + kBlockSz - 1) / kBlockSz]{}},
	          blocks_sz_{(rhs.sz_ + kBlockSz - 1) / kBlockSz}, left_{0}, sz_{0} {
		for(size_t i = 0; i < rhs.sz_; ++i) {
			push_back(rhs[i]);
		}
	}
	Deque(Deque && rhs)
	        : blocks_{rhs.blocks_}, blocks_sz_{rhs.blocks_sz_}, left_{rhs.left_}, sz_{rhs.sz_} {
		rhs.blocks_ = nullptr;
		rhs.blocks_sz_ = 0;
		rhs.left_ = 0;
		rhs.sz_ = 0;
	}
	explicit Deque(size_t size)
	        : blocks_{size == 0 ? nullptr : new int *[(size + kBlockSz - 1) / kBlockSz]{}},
	          blocks_sz_{(size + kBlockSz - 1) / kBlockSz}, left_{0}, sz_{0} {
		for(size_t i = 0; i < size; ++i) {
			push_back(0);
		}
	}

	Deque(std::initializer_list<int> list) : Deque(list.size()) {
		size_t i = 0;
		for(int x : list) {
			(*this)[i++] = x;
		}
	}

	~Deque() {
		clear();
	}

	Deque & operator=(Deque rhs) {
		swap(rhs);
		return *this;
	}

	void swap(Deque & rhs) {
		if(this == &rhs) {
			return;
		}
		std::swap(blocks_, rhs.blocks_);
		std::swap(blocks_sz_, rhs.blocks_sz_);
		std::swap(left_, rhs.left_);
		std::swap(sz_, rhs.sz_);
	}

	void push_back(int value) {
		if(!blocks_sz_) {
			realloc();
		}
		if(sz_) {
			auto [block, p_in_block] = get_pos(sz_);
			auto [front_block, front_pos] = get_pos(0);
			if(block == front_block && p_in_block <= front_pos) {
				realloc();
			}
		}
		auto [block, p_in_block] = get_pos(sz_);
		ensure_block_exist(block);
		blocks_[block][p_in_block] = value;
		++sz_;
	}

	void pop_back() {
		auto [block, p_in_block] = get_pos(sz_ - 1);
		--sz_;
		if(sz_ == 0) {
			left_ = 0;
		}
		if(unused_block(block)) {
			delete[] blocks_[block];
			blocks_[block] = nullptr;
		}
	}

	void push_front(int value) {
		if(!blocks_sz_) {
			realloc();
		}

		size_t new_left = (left_ + blocks_sz_ * kBlockSz - 1) % (blocks_sz_ * kBlockSz);
		auto [block, p_in_block] = split_pos(new_left);

		if(sz_) {
			auto [back_block, back_pos] = get_pos(sz_ - 1);
			if(block == back_block && p_in_block >= back_pos) {
				realloc();

				new_left = (left_ + blocks_sz_ * kBlockSz - 1) %
				           (blocks_sz_ * kBlockSz);
				auto tmp = split_pos(new_left);
				block = tmp.first;
				p_in_block = tmp.second;
			}
		}

		ensure_block_exist(block);
		blocks_[block][p_in_block] = value;
		left_ = new_left;
		++sz_;
	}

	void pop_front() {
		auto [block, p_in_block] = get_pos(0);
		--sz_;
		if(sz_ != 0) {
			left_ = (left_ + 1) % (blocks_sz_ * kBlockSz);
		} else {
			left_ = 0;
		}
		if(unused_block(block)) {
			delete[] blocks_[block];
			blocks_[block] = nullptr;
		}
	}

	int & operator[](size_t ind) {
		auto [block, p_in_block] = get_pos(ind);
		return blocks_[block][p_in_block];
	}

	int operator[](size_t ind) const {
		auto [block, p_in_block] = get_pos(ind);
		return blocks_[block][p_in_block];
	}
	size_t size() const {
		return sz_;
	}

	void clear() {
		for(size_t i = 0; i < blocks_sz_; ++i) {
			delete[] blocks_[i];
		}
		delete[] blocks_;
		blocks_ = nullptr;
		blocks_sz_ = 0;
		sz_ = 0;
		left_ = 0;
	}

    private:
	static constexpr size_t kBlockSz = 512 / sizeof(int);
	int ** blocks_;
	size_t blocks_sz_;
	size_t left_;
	size_t sz_;

	void ensure_block_exist(size_t block) {
		if(!blocks_[block]) {
			blocks_[block] = new int[kBlockSz]{};
		}
	}

	std::pair<size_t, size_t> get_pos(size_t idx) const {
		size_t cap = blocks_sz_ * kBlockSz;
		size_t i = (left_ + idx) % cap;
		return {i / kBlockSz, i % kBlockSz};
	}

	void realloc() {
		if(blocks_sz_ == 0) {
			blocks_ = new int *[1]{};
			blocks_sz_ = 1;
			left_ = 0;
			return;
		}
		int ** new_blocks = new int *[blocks_sz_ * 2]{};
		size_t l_block = get_pos(0).first;
		for(size_t i = 0; i < blocks_sz_; ++i) {
			new_blocks[i] = blocks_[(l_block + i) % blocks_sz_];
		}
		delete[] blocks_;
		blocks_ = new_blocks;
		blocks_sz_ *= 2;
		left_ %= kBlockSz;
	}

	bool unused_block(size_t block) {
		if(sz_ == 0) {
			return true;
		}
		size_t l_block = get_pos(0).first;
		size_t r_block = get_pos(sz_ - 1).first;

		return block != l_block && block != r_block;
	}

	std::pair<size_t, size_t> split_pos(size_t p) const {
		return {p / kBlockSz, p % kBlockSz};
	}
};
