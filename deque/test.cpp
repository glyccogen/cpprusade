#include "../.for_tests/catch.hpp"

#include <vector>
#include <random>
#include <deque>

#include "deque.h"

void check(const Deque& actual, const std::vector<int>& expected) {
    REQUIRE(actual.size() == expected.size());
    for (size_t i = 0; i < actual.size(); ++i) {
        REQUIRE(actual[i] == expected[i]);
    }
}

TEST_CASE("Deque has constructors", "[deque]") {
    {
        Deque a;
        REQUIRE(a.size() == 0u);
    }
    {
        Deque a{1, 2, 3, 4};
        check(a, std::vector<int>{1, 2, 3, 4});
    }
    {
        Deque a(5);
        check(a, std::vector<int>(5));
    }
}

TEST_CASE("Basic methods", "[deque]") {
    Deque a{1, 3, 5};
    check(a, std::vector<int>{1, 3, 5});

    a.pop_back();
    check(a, std::vector<int>{1, 3});
    a.pop_front();
    check(a, std::vector<int>{3});
    a.push_front(5);
    check(a, std::vector<int>{5, 3});
    a.push_back(1);
    check(a, std::vector<int>{5, 3, 1});

    a.clear();
    check(a, std::vector<int>());

    a.push_back(3);
    Deque b{2, 4};
    a.swap(b);
    check(a, std::vector<int>{2, 4});
    check(b, std::vector<int>{3});
}

TEST_CASE("Modifications with []", "[deque]") {
    Deque a{9, 1, 1};
    a[0] = 1;
    a[1] = 2;
    a[2] = 3;
    check(a, std::vector<int>{1, 2, 3});
}

TEST_CASE("Memory layout", "[vector]") {
    Deque a(129);
    for (int i = 0; i < 127; ++i) {
        REQUIRE(&a[i] + 1 == &a[i + 1]);
    }
}

TEST_CASE("Reallocations", "[deque]") {
    Deque a;
    const int iterations = 1e6;
    std::vector<int*> addr;
    for (int i = 0; i < iterations; ++i) {
        a.push_back(i);
        addr.push_back(&a[i]);
    }

    for (int i = 0; i < iterations; ++i) {
        REQUIRE(*addr[i] == i);
    }
}

TEST_CASE("Copy correctness", "[vector]") {
    Deque a;
    Deque b(a);
    b.push_back(1);
    check(a, std::vector<int>());
    check(b, std::vector<int>{1});

    b = b;  // NOLINT
    check(b, std::vector<int>{1});
    a = b;
    check(a, std::vector<int>{1});

    b = std::move(a);
    check(b, std::vector<int>{1});
    Deque c(std::move(b));
    check(c, std::vector<int>{1});

    Deque d{3, 4, 5};
    Deque e(d);
    check(e, std::vector<int>{3, 4, 5});
    d.swap(c);
    check(e, std::vector<int>{3, 4, 5});
    check(d, std::vector<int>{1});
    check(c, std::vector<int>{3, 4, 5});
}

TEST_CASE("Stress", "[deque]") {
    const int iterations = 1e6;
    Deque a;
    std::deque<int> b;
    std::mt19937 gen(735675);
    std::uniform_int_distribution<int> dist(1, 5);

    for (int i = 0; i < iterations; ++i) {
        a.push_front(i);
        b.push_front(i);
    }

    for (int i = 0; i < iterations; ++i) {
        int code = dist(gen);
        int value = gen();
        if (code == 1) {
            a.push_front(value);
            b.push_front(value);
        } else if (code == 2) {
            a.push_back(value);
            b.push_back(value);
        } else if (code == 3) {
            a.pop_front();
            b.pop_front();
        } else if (code == 4) {
            a.pop_back();
            b.pop_back();
        } else {
            int index = static_cast<int>(value % a.size());
            REQUIRE(a[index] == b[index]);
        }
    }
}

TEST_CASE("Empty correctness") {
    // There are some ways to make deque empty
    // We should test them all
    // In some ways we can cause memory leak
    const size_t test_size = 1e3;
    {
        // PushBack-PopBack case
        Deque a;
        for (size_t idx = 0; idx < test_size; ++idx) {
            a.push_back(idx);
        }
        for (size_t idx = 0; idx < test_size; ++idx) {
            a.pop_back();
        }
        REQUIRE(a.size() == 0u);
        check(a, std::vector<int>());
    }

    {
        // PushBack-PopFront case
        // this case is broken in my implementation
        Deque a;
        for (size_t idx = 0; idx < test_size; ++idx) {
            a.push_back(idx);
        }
        for (size_t idx = 0; idx < test_size; ++idx) {
            a.pop_front();
        }
        REQUIRE(a.size() == 0u);
        check(a, std::vector<int>());
    }

    {
        // PushFront-PopBack case
        Deque a;
        for (size_t idx = 0; idx < test_size; ++idx) {
            a.push_front(idx);
        }
        for (size_t idx = 0; idx < test_size; ++idx) {
            a.pop_back();
        }
        REQUIRE(a.size() == 0u);
        check(a, std::vector<int>());
    }

    {
        // PushFront-PopFront case
        Deque a;
        for (size_t idx = 0; idx < test_size; ++idx) {
            a.push_front(idx);
        }
        for (size_t idx = 0; idx < test_size; ++idx) {
            a.pop_front();
        }
        REQUIRE(a.size() == 0u);
        check(a, std::vector<int>());
    }
}

TEST_CASE("Correct work of cycled buffer") {
    Deque a;
    a.push_front(1);
    a.push_back(2);
    check(a, std::vector<int>{1, 2});

    Deque b;
    b.push_back(1);
    b.push_front(2);
    check(b, std::vector<int>{2, 1});

    // Check cycled buffer work after reallocation
    const int iterations = 128;

    std::vector<int> v{1, 2};
    for (int i = 0; i < iterations; ++i) {
        a.push_back(i);
        v.push_back(i);
    }
    check(a, v);

    std::deque<int> w{2, 1};
    for (int i = 0; i < iterations; ++i) {
        b.push_front(i);
        w.push_front(i);
    }
    check(b, std::vector<int>(w.begin(), w.end()));
}

TEST_CASE("Front and back traversing the same block") {
    Deque a;

    // Traverse a single block
    const int iterations = 128;

    // PushBack => PopFront
    for (int i = 0; i < iterations; ++i) {
        a.push_back(i);
        REQUIRE(a[0] == i);
        a.pop_front();
    }
    REQUIRE(a.size() == 0u);

    // PushFront => PopBack
    for (int i = 0; i < iterations; ++i) {
        a.push_front(i);
        REQUIRE(a[0] == i);
        a.pop_back();
    }
    REQUIRE(a.size() == 0u);
}
