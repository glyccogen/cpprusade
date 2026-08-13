#pragma once

#include <initializer_list>
#include <algorithm>
#include <deque>

class Deque {
public:
    Deque() = default;
    Deque(const Deque& rhs) = default;
    Deque(Deque&& rhs) = default;
    explicit Deque(size_t size) : data_(size) {
    }

    Deque(std::initializer_list<int> list) : data_(list) {
    }

    Deque& operator=(Deque rhs) {
        swap(rhs);
        return *this;
    }

    void swap(Deque& rhs) {
        std::swap(data_, rhs.data_);
    }

    void push_back(int value) {
        data_.push_back(value);
    }

    void pop_back() {
        data_.pop_back();
    }

    void push_front(int value) {
        data_.push_front(value);
    }

    void pop_front() {
        data_.pop_front();
    }

    int& operator[](size_t ind) {
        return data_[ind];
    }

    int operator[](size_t ind) const {
        return data_[ind];
    }

    size_t size() const {
        return data_.size();
    }

    void clear() {
        data_.clear();
    }

private:
    std::deque<int> data_;
};
