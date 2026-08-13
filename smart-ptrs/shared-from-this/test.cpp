#include "shared.h"
#include "weak.h"

#include "../../.for_tests/catch.hpp"

struct T : public EnableSharedFromThis<T> {};

struct Y : T {};

struct Z : Y {};

void nullDeleter(void *) {}

struct Foo : virtual public EnableSharedFromThis<Foo> {
	virtual ~Foo() {}
};

struct Bar : public Foo {
	Bar(int) {}
};

TEST_CASE("SharedFromThis") {
	{
		SharedPtr<T> t1(new T);
		SharedPtr<T> t2(makeShared<T>());
	}

	{
		int x = 42;
		SharedPtr<Bar> t1(new Bar(42));
		REQUIRE(t1->shared_from_this() == t1);
		SharedPtr<Bar> t2(makeShared<Bar>(x));
		REQUIRE(t2->shared_from_this() == t2);
	}

	{
		SharedPtr<Y> p(new Z);
		SharedPtr<T> q = p->shared_from_this();
		REQUIRE(p == q);
	}

	{
		T * ptr = new T;
		SharedPtr<T> s(ptr);
		REQUIRE(!ptr->weak_from_this().expired());
		{
			try {
				SharedPtr<T> new_s = ptr->shared_from_this();
				REQUIRE(new_s == s);
			} catch(...) {
				REQUIRE(false);
			}
		}
		s.reset();
	}

	{
		T * ptr = new T;
		WeakPtr<T> weak;
		{
			SharedPtr<T> s(ptr);
			REQUIRE(ptr->shared_from_this() == s);
			weak = s;
			REQUIRE(!weak.expired());
		}
		REQUIRE(weak.expired());
		weak.reset();
	}
}

TEST_CASE("WeakFromThis") {
	T * ptr = new T;
	T const * cptr = ptr;

	static_assert(noexcept(ptr->weak_from_this()), "Operation must be noexcept");
	static_assert(noexcept(cptr->weak_from_this()), "Operation must be noexcept");

	WeakPtr<T> weak = ptr->weak_from_this();
	REQUIRE(weak.expired());

	WeakPtr<T const> my_const_weak = cptr->weak_from_this();
	REQUIRE(my_const_weak.expired());

	SharedPtr<T> sptr(ptr);
	weak = ptr->weak_from_this();
	REQUIRE(!weak.expired());
	REQUIRE(weak.lock().get() == ptr);
}
