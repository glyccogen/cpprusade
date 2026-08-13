#include "shared.h"
#include "weak.h"

#include "../../.for_tests/my_int.h"

#include "../../.for_tests/catch.hpp"

#include "../../.for_tests/allocations_checker.h"

#include <string>
#include <utility>

////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("Empty weak") {
	WeakPtr<int> a;
	WeakPtr<int> b;
	a = b;
	WeakPtr c(a);
	b = std::move(c);

	auto shared = b.lock();
	REQUIRE(shared.get() == nullptr);
}

TEST_CASE("No unexpected allocations") {
	EXPECT_ZERO_ALLOCATIONS(WeakPtr<int>{});

	auto sp = makeShared<int>(42);
	EXPECT_ZERO_ALLOCATIONS(WeakPtr<int>{sp});

	SharedPtr<int> sp2(new int(42));
	EXPECT_ZERO_ALLOCATIONS(WeakPtr<int>{sp2});
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("Copy/move WeakPtr") {
	SharedPtr<std::string> a(new std::string("aba"));
	WeakPtr<std::string> b(a);
	WeakPtr<std::string> empty;
	WeakPtr c(b);
	WeakPtr<std::string> d(a);

	REQUIRE(d.use_count() == 1);

	REQUIRE(!c.expired());
	c = empty;
	REQUIRE(c.expired());

	b = std::move(c);

	WeakPtr e(std::move(d));
	REQUIRE(d.lock().get() == nullptr);

	auto locked = e.lock();
	REQUIRE(*locked == "aba");

	WeakPtr<std::string> start(a);
	{
		SharedPtr a2(a);
		WeakPtr<std::string> f(a2);
		auto cur_lock = f.lock();
		REQUIRE(cur_lock.get() == SharedPtr(start).get());
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("Modifiers WeakPtr") {
	SECTION("reset") {
		{
			SharedPtr<int> shared = makeShared<int>(42), shared2 = shared,
			               shared3 = shared2;
			WeakPtr<int> weak = WeakPtr<int>{shared};
			REQUIRE(shared.use_count() == 3);
			REQUIRE(weak.use_count() == 3);
			REQUIRE(!weak.expired());
			weak.reset();
			REQUIRE(shared.use_count() == 3);
			REQUIRE(weak.use_count() == 0);
			REQUIRE(weak.expired());
		}
	}

	SECTION("reset deletes block") {
		WeakPtr<int> * wp;
		{
			auto sp = makeShared<int>();
			wp = new WeakPtr<int>(sp);
		}
		wp->reset();
		delete wp;
	}

	SECTION("swap") {
		{
			SharedPtr<int> shared = makeShared<int>(42), shared3 = shared;
			SharedPtr<int> shared2 = makeShared<int>(13);
			WeakPtr<int> weak = WeakPtr<int>{shared};
			WeakPtr<int> weak2 = WeakPtr<int>{shared2};
			REQUIRE(weak.use_count() == 2);
			REQUIRE(weak2.use_count() == 1);
			weak.swap(weak2);
			REQUIRE(weak.use_count() == 1);
			REQUIRE(weak2.use_count() == 2);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("Weak expiration") {
	WeakPtr<std::string> * a;
	{
		SharedPtr<std::string> b(new std::string("aba"));
		SharedPtr c(b);
		a = new WeakPtr<std::string>(c);
		auto test = a->lock();
		REQUIRE(*test == "aba");
		REQUIRE(!a->expired());
	}
	REQUIRE(a->expired());
	delete a;
}

TEST_CASE("Weak extends Shared") {
	SharedPtr<std::string> * b = new SharedPtr<std::string>(new std::string("aba"));
	WeakPtr<std::string> c(*b);
	auto a = c.lock();
	delete b;
	REQUIRE(!c.expired());
	REQUIRE(*a == "aba");
}

TEST_CASE("Shared from Weak") {
	SharedPtr<std::string> * x = new SharedPtr<std::string>(new std::string("aba"));
	WeakPtr<std::string> y(*x);
	delete x;
	REQUIRE(y.expired());
	SharedPtr z = y.lock();
	REQUIRE(z.get() == nullptr);
}

TEST_CASE("Shared from invalid Weak") {
	WeakPtr<int> w_ptr;
	{
		SharedPtr<int> ptr = makeShared<int>(42);
		w_ptr = ptr;
	}
	REQUIRE_THROWS_AS(SharedPtr<int>(w_ptr), BadWeakPtr);
}

TEST_CASE("Constness") {
	SharedPtr<int> sp(new int(42));
	WeakPtr<int const> wp(sp);
}

TEST_CASE("Lifetimes") {
	SECTION("Destructor is called in time") {
		WeakPtr<MyInt> * wp;
		{
			auto sp = makeShared<MyInt>();

			REQUIRE(MyInt::alive_count() == 1);

			wp = new WeakPtr<MyInt>(sp);
		}
		REQUIRE(MyInt::alive_count() == 0);
		delete wp;
	}

	SECTION("Destructor is called once") {
		WeakPtr<std::string> * wp;
		{
			auto sp = makeShared<std::string>("looooooooooooooooooooooooooong");
			wp = new WeakPtr<std::string>(sp);
		}
		delete wp;
	}
}
