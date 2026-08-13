#include "shared.h"

#include "../../.for_tests/catch.hpp"

#include "../../.for_tests/allocations_checker.h"

#include <memory>
#include <string>
#include <type_traits>
#include <utility>

////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("Empty") {
	SECTION("Empty state") {
		SharedPtr<int> a, b;

		b = a;
		SharedPtr c(a);
		b = std::move(c);

		REQUIRE(a.get() == nullptr);
		REQUIRE(b.get() == nullptr);
		REQUIRE(c.get() == nullptr);
	}

	SECTION("No allocations in default ctor") {
		EXPECT_ZERO_ALLOCATIONS(SharedPtr<int>());
		EXPECT_ZERO_ALLOCATIONS(SharedPtr<int>(nullptr));
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("Copy/move") {
	SharedPtr<std::string> a(new std::string("aba"));
	std::string * ptr;
	{
		SharedPtr b(a);
		SharedPtr c(a);
		ptr = c.get();
	}
	REQUIRE(ptr == a.get());
	REQUIRE(*ptr == "aba");

	SharedPtr<std::string> b(new std::string("caba"));
	{
		SharedPtr c(b);
		SharedPtr d(b);
		d = std::move(a);
		REQUIRE(*c == "caba");
		REQUIRE(*d == "aba");
		b.reset(new std::string("test"));
		REQUIRE(*c == "caba");
	}
	REQUIRE(*b == "test");

	SharedPtr<std::string> end;
	{
		SharedPtr<std::string> d(new std::string("delete"));
		d = b;
		SharedPtr c(std::move(b));
		REQUIRE(*d == "test");
		REQUIRE(*c == "test");
		d = d; // NOLINT
		c = end;
		d.reset(new std::string("delete"));
		end = d;
	}

	REQUIRE(*end == "delete");
}

////////////////////////////////////////////////////////////////////////////////////////////////////

struct ModifiersB {
	static int count;

	ModifiersB() {
		++count;
	}
	ModifiersB(ModifiersB const &) {
		++count;
	}
	virtual ~ModifiersB() {
		--count;
	}
};

int ModifiersB::count = 0;

struct ModifiersA : public ModifiersB {
	static int count;

	ModifiersA() {
		++count;
	}
	ModifiersA(ModifiersA const & other) : ModifiersB(other) {
		++count;
	}
	~ModifiersA() {
		--count;
	}
};

int ModifiersA::count = 0;

struct ModifiersC {
	static int count;

	ModifiersC() {
		++count;
	}
	ModifiersC(ModifiersC const &) {
		++count;
	}
	~ModifiersC() {
		--count;
	}
};

int ModifiersC::count = 0;

TEST_CASE("Modifiers") {
	SECTION("reset") {
		{
			SharedPtr<ModifiersB> p(new ModifiersB);
			p.reset();
			REQUIRE(ModifiersA::count == 0);
			REQUIRE(ModifiersB::count == 0);
			REQUIRE(p.use_count() == 0);
			REQUIRE(p.get() == nullptr);
		}
		REQUIRE(ModifiersA::count == 0);
		{
			SharedPtr<ModifiersB> p;
			p.reset();
			REQUIRE(ModifiersA::count == 0);
			REQUIRE(ModifiersB::count == 0);
			REQUIRE(p.use_count() == 0);
			REQUIRE(p.get() == nullptr);
		}
		REQUIRE(ModifiersA::count == 0);
	}

	SECTION("reset ptr") {
		{
			SharedPtr<ModifiersB> p(new ModifiersB);
			ModifiersA * ptr = new ModifiersA;
			p.reset(ptr);
			REQUIRE(ModifiersA::count == 1);
			REQUIRE(ModifiersB::count == 1);
			REQUIRE(p.use_count() == 1);
			REQUIRE(p.get() == ptr);
		}
		REQUIRE(ModifiersA::count == 0);
		{
			SharedPtr<ModifiersB> p;
			ModifiersA * ptr = new ModifiersA;
			p.reset(ptr);
			REQUIRE(ModifiersA::count == 1);
			REQUIRE(ModifiersB::count == 1);
			REQUIRE(p.use_count() == 1);
			REQUIRE(p.get() == ptr);
		}
		REQUIRE(ModifiersA::count == 0);
	}

	SECTION("swap") {
		{
			ModifiersC * ptr1 = new ModifiersC;
			ModifiersC * ptr2 = new ModifiersC;
			SharedPtr<ModifiersC> p1(ptr1);
			{
				SharedPtr<ModifiersC> p2(ptr2);
				p1.swap(p2);
				REQUIRE(p1.use_count() == 1);
				REQUIRE(p1.get() == ptr2);
				REQUIRE(p2.use_count() == 1);
				REQUIRE(p2.get() == ptr1);
				REQUIRE(ModifiersC::count == 2);
			}
			REQUIRE(p1.use_count() == 1);
			REQUIRE(p1.get() == ptr2);
			REQUIRE(ModifiersC::count == 1);
		}
		REQUIRE(ModifiersC::count == 0);
		{
			ModifiersC * ptr1 = new ModifiersC;
			ModifiersC * ptr2 = nullptr;
			SharedPtr<ModifiersC> p1(ptr1);
			{
				SharedPtr<ModifiersC> p2;
				p1.swap(p2);
				REQUIRE(p1.use_count() == 0);
				REQUIRE(p1.get() == ptr2);
				REQUIRE(p2.use_count() == 1);
				REQUIRE(p2.get() == ptr1);
				REQUIRE(ModifiersC::count == 1);
			}
			REQUIRE(p1.use_count() == 0);
			REQUIRE(p1.get() == ptr2);
			REQUIRE(ModifiersC::count == 0);
		}
		REQUIRE(ModifiersC::count == 0);
		{
			ModifiersC * ptr1 = nullptr;
			ModifiersC * ptr2 = new ModifiersC;
			SharedPtr<ModifiersC> p1;
			{
				SharedPtr<ModifiersC> p2(ptr2);
				p1.swap(p2);
				REQUIRE(p1.use_count() == 1);
				REQUIRE(p1.get() == ptr2);
				REQUIRE(p2.use_count() == 0);
				REQUIRE(p2.get() == ptr1);
				REQUIRE(ModifiersC::count == 1);
			}
			REQUIRE(p1.use_count() == 1);
			REQUIRE(p1.get() == ptr2);
			REQUIRE(ModifiersC::count == 1);
		}
		REQUIRE(ModifiersC::count == 0);
		{
			ModifiersC * ptr1 = nullptr;
			ModifiersC * ptr2 = nullptr;
			SharedPtr<ModifiersC> p1;
			{
				SharedPtr<ModifiersC> p2;
				p1.swap(p2);
				REQUIRE(p1.use_count() == 0);
				REQUIRE(p1.get() == ptr2);
				REQUIRE(p2.use_count() == 0);
				REQUIRE(p2.get() == ptr1);
				REQUIRE(ModifiersC::count == 0);
			}
			REQUIRE(p1.use_count() == 0);
			REQUIRE(p1.get() == ptr2);
			REQUIRE(ModifiersC::count == 0);
		}
		REQUIRE(ModifiersC::count == 0);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////

struct OperatorBoolA {
	int a;
	virtual ~OperatorBoolA() {};
};

TEST_CASE("Observers") {
	SECTION("operator->") {
		SharedPtr<std::pair<int, int>> const p(new std::pair<int, int>(3, 4));
		REQUIRE(p->first == 3);
		REQUIRE(p->second == 4);
		p->first = 5;
		p->second = 6;
		REQUIRE(p->first == 5);
		REQUIRE(p->second == 6);
	}

	SECTION("Dereference") {
		SharedPtr<int> const p(new int(32));
		REQUIRE(*p == 32);
		*p = 3;
		REQUIRE(*p == 3);
	}

	SECTION("operator bool") {
		static_assert(std::is_constructible<bool, SharedPtr<OperatorBoolA>>::value, "");
		static_assert(!std::is_convertible<SharedPtr<OperatorBoolA>, bool>::value, "");
		{
			SharedPtr<int> const p(new int(32));
			REQUIRE(p);
		}
		{
			SharedPtr<int> const p;
			REQUIRE(!p);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////

struct Pinned {
	Pinned(int tag) : tag_(tag) {}

	Pinned(Pinned const & a) = delete;
	Pinned(Pinned && a) = delete;

	Pinned & operator=(Pinned const & a) = delete;
	Pinned & operator=(Pinned && a) = delete;

	~Pinned() = default;

	int get_tag() const {
		return tag_;
	}

    private:
	int tag_;
};

TEST_CASE("No copies") {
	SharedPtr<Pinned> p(new Pinned(1));
}

struct D {
	D(Pinned & pinned, std::unique_ptr<int> && p)
	        : some_uncopyable_thing_(std::move(p)), pinned_(pinned) {}

	int get_up() const {
		return *some_uncopyable_thing_;
	}

	Pinned & get_pinned() const {
		return pinned_;
	}

    private:
	std::unique_ptr<int> some_uncopyable_thing_;
	Pinned & pinned_;
};

struct Throwing {
	Throwing() {
		throw 42;
	}
};

TEST_CASE("makeShared") {
	SECTION("One allocation") {
		EXPECT_ONE_ALLOCATION(REQUIRE(*makeShared<int>(42) == 42));
	}

	SECTION("Parameters passing") {
		auto p_int = std::make_unique<int>(42);
		Pinned pinned(1312);
		auto p = makeShared<D>(pinned, std::move(p_int));

		REQUIRE(p->get_up() == 42);
		REQUIRE(p->get_pinned().get_tag() == 1312);
	}

	SECTION("Constructed only once") {
		auto sp = makeShared<Pinned>(1);
	}

	SECTION("Faulty constructor") {
		try {
			auto sp = makeShared<Throwing>();
		} catch(...) {
		}
	}
}

struct Data {
	static bool data_was_deleted;

	int x;
	double y;

	~Data() {
		data_was_deleted = true;
	}
};

bool Data::data_was_deleted = false;

TEST_CASE("Aliasing constructor") {
	SECTION("It just exists") {
		SharedPtr<Data> sp(new Data{42, 3.14});

		SharedPtr<double> sp2(sp, &sp->y);

		REQUIRE(*sp2 == 3.14);
	}

	SECTION("Lifetime extension") {
		{
			Data::data_was_deleted = false;
			SharedPtr<double> sp3;
			{
				SharedPtr<Data> sp(new Data{42, 3.14});
				SharedPtr<double> sp2(sp, &sp->y);
				sp3 = sp2;
			}
			REQUIRE(*sp3 == 3.14);
			REQUIRE(!Data::data_was_deleted);
		}
		REQUIRE(Data::data_was_deleted);
	}
}

class Base {
    public:
	virtual ~Base() = default;
};

class Derived : public Base {
    public:
	static bool i_was_deleted;

	~Derived() {
		i_was_deleted = true;
	}
};

bool Derived::i_was_deleted = false;

TEST_CASE("Type conversions") {
	SECTION("Destruction") {
		Derived::i_was_deleted = false;
		{
			SharedPtr<Base> sb(new Derived);
		}
		REQUIRE(Derived::i_was_deleted);
	}

	SECTION("Constness") {
		SharedPtr<int> s1(new int(42));
		SharedPtr<int const> s2 = s1;

		SharedPtr<int const> s3 = std::move(s1);
		REQUIRE(!s1);
		REQUIRE(s2.use_count() == 2);

		s1.reset(new int(43));
		s2 = s1;
		s3 = std::move(s1);
		REQUIRE(!s1);
		REQUIRE(s3.use_count() == 2);
	}
}

struct A {
	~A() = default;
};

struct B : A {
	~B() {
		destructor_called = true;
	}

	static bool destructor_called;
};

bool B::destructor_called = false;

TEST_CASE("Destructor for correct type") {
	SECTION("Regular constructor") {
		B::destructor_called = false;
		{
			SharedPtr<A>(new B());
		}
		REQUIRE(B::destructor_called);
	}

	SECTION("makeShared") {
		B::destructor_called = false;
		{
			SharedPtr<A> ptr = makeShared<B>();
		}
		REQUIRE(B::destructor_called);
	}

	SECTION("reset") {
		B::destructor_called = false;
		{
			SharedPtr<A> ptr(new A);
			ptr.reset(new B);
		}
		REQUIRE(B::destructor_called);
	}
}
