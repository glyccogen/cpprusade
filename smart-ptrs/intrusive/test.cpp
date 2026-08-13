#include "intrusive.h"

#include "../../.for_tests/catch.hpp"

#include "../../.for_tests/allocations_checker.h"

#include <string>

////////////////////////////////////////////////////////////////////////////////

struct MyInt : public SimpleRefCounted<MyInt> {
	MyInt(int value) : value{value} {}

	int value = 0;
};

struct MyString : public SimpleRefCounted<MyString>, public std::string {
	using std::string::basic_string;
};

TEST_CASE("Empty") {
	SECTION("Sizeof") {
		REQUIRE(sizeof(IntrusivePtr<MyInt>) == sizeof(void *));
	}

	SECTION("Empty state") {
		IntrusivePtr<MyInt> a, b;

		b = a;
		IntrusivePtr c(a);
		b = std::move(c);

		REQUIRE(a.get() == nullptr);
		REQUIRE(b.get() == nullptr);
		REQUIRE(c.get() == nullptr);
	}
}

TEST_CASE("Copy/move") {
	SECTION("Constructors") {
		IntrusivePtr<MyString> a{new MyString{"abacaba"}};
		IntrusivePtr<MyString> b = a;
		IntrusivePtr<MyString> c = std::move(a);
		IntrusivePtr<MyString> d = b;
		REQUIRE(c.use_count() == 3);
		REQUIRE(!a);
		REQUIRE(*b == "abacaba");
		REQUIRE(*c == "abacaba");
		REQUIRE(*d == "abacaba");
	}

	SECTION("Assignment") {
		IntrusivePtr<MyString> a{new MyString{"abracadabra"}};
		IntrusivePtr<MyString> b{new MyString{"karabas"}};
		IntrusivePtr<MyString> c = b;
		b = a;
		a = b;
		b = a;
		a = b;
		c = c;            // NOLINT
		c = std::move(c); // NOLINT
		b = std::move(b); // NOLINT
		REQUIRE(*a == "abracadabra");
		REQUIRE(*b == "abracadabra");
		REQUIRE(*c == "karabas");
	}

	SECTION("Mulitple copies") {
		std::vector<IntrusivePtr<MyString>> ptrs;
		ptrs.emplace_back(new MyString{"hehe"});

		constexpr int kNumIters = 1000;
		for(size_t i = kNumIters; i-- > 0;) {
			ptrs.emplace_back(ptrs.back());
		}
		for(auto && ptr : ptrs) {
			REQUIRE(ptr.use_count() == 1 + kNumIters);
			REQUIRE(ptr.get() == ptrs.back().get());
		}

		ptrs.resize(5);
		for(auto && ptr : ptrs) {
			REQUIRE(ptr.use_count() == 5);
			REQUIRE(ptr.get() == ptrs.back().get());
		}
	}
}

TEST_CASE("Conversions") {
	struct Foo : SimpleRefCounted<Foo> {
		virtual ~Foo() = default;
		virtual int kek() = 0;
	};

	struct Boo : Foo {
		int value;

		Boo(int value) : value{value} {}

		int kek() override {
			return value;
		}
	};

	IntrusivePtr<Boo> boo = makeIntrusive<Boo>(123);
	IntrusivePtr<Foo> foo = boo;
	foo = makeIntrusive<Boo>(42);
	REQUIRE(foo->kek() == 42);
}

template <typename T> class ObjectCounters {
    public:
	ObjectCounters() {
		++created;
		++alive;
	}

	~ObjectCounters() {
		--alive;
	}

	static size_t num_alive() {
		return alive;
	}

	static size_t num_created() {
		return created;
	}

	static void reset_counters() {
		created = 0;
		alive = 0;
	}

    private:
	static inline size_t created = 0;
	static inline size_t alive = 0;
};

struct CountedString : std::string, ObjectCounters<CountedString>, SimpleRefCounted<CountedString> {
	using std::string::basic_string;
};

TEST_CASE("Modifiers") {
	SECTION("reset()") {
		IntrusivePtr<CountedString> p{new CountedString{}};
		REQUIRE(CountedString::num_alive() == 1);

		p.reset();
		REQUIRE(CountedString::num_alive() == 0);
		REQUIRE(p.use_count() == 0);
		REQUIRE(p.get() == nullptr);

		p.reset();
		REQUIRE(CountedString::num_alive() == 0);
		REQUIRE(p.use_count() == 0);
		REQUIRE(p.get() == nullptr);
	}

	CountedString::reset_counters();

	SECTION("reset(T*)") {
		IntrusivePtr<CountedString> p{new CountedString{"boo"}};
		REQUIRE(CountedString::num_created() == 1);
		p.reset(new CountedString{"foo"});
		REQUIRE(CountedString::num_created() == 2);
		REQUIRE(CountedString::num_alive() == 1);
		REQUIRE(*p == "foo");
	}

	CountedString::reset_counters();

	SECTION("swap") {
		IntrusivePtr<CountedString> p{new CountedString{"first"}};
		IntrusivePtr<CountedString> q{new CountedString{"second"}};
		REQUIRE(CountedString::num_created() == 2);
		REQUIRE(CountedString::num_alive() == 2);

		p.swap(p);
		REQUIRE(CountedString::num_created() == 2);
		REQUIRE(CountedString::num_alive() == 2);
		REQUIRE(*p == "first");
		REQUIRE(*q == "second");

		p.swap(q);
		REQUIRE(CountedString::num_created() == 2);
		REQUIRE(CountedString::num_alive() == 2);
		REQUIRE(*p == "second");
		REQUIRE(*q == "first");
	}
}

TEST_CASE("Observers") {
	struct IntrusivePair : SimpleRefCounted<IntrusivePair> {
		int first;
		int second;
	};

	SECTION("operator->") {
		IntrusivePtr<IntrusivePair> const p(new IntrusivePair{.first = 3, .second = 4});
		REQUIRE(p->first == 3);
		REQUIRE(p->second == 4);
		p->first = 5;
		p->second = 6;
		REQUIRE(p->first == 5);
		REQUIRE(p->second == 6);
	}

	SECTION("Dereference") {
		IntrusivePtr<MyInt> const p(new MyInt(32));
		REQUIRE((*p).value == 32);
		*p = 3;
		REQUIRE((*p).value == 3);
	}

	SECTION("Dereference 2") {
		IntrusivePtr<MyInt> const p(new MyInt(24));
		REQUIRE((*p).value == 24);
		*p = 1;
		IntrusivePtr<MyInt> ptr1(p);
		IntrusivePtr<MyInt> ptr2(p);
		IntrusivePtr<MyInt> ptr3(p);
		ptr2 = std::move(ptr3);
		REQUIRE((*p).value == 1);
	}

	SECTION("operator bool") {
		static_assert(std::is_constructible<bool, IntrusivePtr<MyString>>::value, "");
		static_assert(!std::is_convertible<IntrusivePtr<MyString>, bool>::value, "");
		{
			IntrusivePtr<MyString> const p(new MyString("kek"));
			REQUIRE(p);
		}
		{
			IntrusivePtr<MyString> const p;
			REQUIRE(!p);
		}
	}
}

TEST_CASE("From raw pointer") {
	MyString * str = new MyString{"Molodoy Krakodil khochet zavesti sebe druzey"};
	IntrusivePtr<MyString> a{str};
	IntrusivePtr<MyString> b{str};
	IntrusivePtr<MyString> c{a};
	IntrusivePtr<MyString> d{c.get()};
	REQUIRE(str->ref_count() == 4);
}

struct Pinned : SimpleRefCounted<Pinned> {
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
	IntrusivePtr<Pinned> p(new Pinned(1));
}

template <typename T> class ObjectInPool;

template <typename T> class ObjectPool {
	static_assert(std::is_base_of_v<ObjectInPool<T>, T>, "Unsupported type");

    public:
	template <typename... Args> IntrusivePtr<T> allocate(Args &&... args) {
		if(!objects_.empty()) {
			std::unique_ptr<T> ptr = std::move(objects_.back());
			objects_.pop_back();
			return IntrusivePtr<T>(ptr.release());
		}
		return do_allocate(std::forward<Args>(args)...);
	}

	void release(T * ptr) {
		objects_.emplace_back(ptr);
	}

	size_t num_available() const {
		return objects_.size();
	}

	size_t num_in_use() const {
		return allocated_ - num_available();
	}

    private:
	template <typename... Args> IntrusivePtr<T> do_allocate(Args &&... args) {
		++allocated_;
		std::unique_ptr<T> object = std::make_unique<T>(std::forward<Args>(args)...);
		object->set_home(this);
		return IntrusivePtr<T>(object.release());
	}

    private:
	std::vector<std::unique_ptr<T>> objects_;
	size_t allocated_ = 0;
};

template <typename Derived> class ObjectInPool {
    public:
	void inc_ref() {
		count_++;
	}

	void dec_ref() {
		if(--count_ == 0) {
			take_me_home();
		}
	}

	size_t ref_count() const {
		return count_;
	}

	void set_home(ObjectPool<Derived> * pool) {
		home_ = pool;
	}

    private:
	void take_me_home() {
		home_->release(static_cast<Derived *>(this));
	}

    private:
	size_t count_ = 0;
	ObjectPool<Derived> * home_;
};

struct PoolableString : ObjectInPool<PoolableString>, std::string {
	using std::string::basic_string;
};

TEST_CASE("Object pool") {
	ObjectPool<PoolableString> strs;

	SECTION("Simple") {
		strs.allocate("first");
		REQUIRE(*strs.allocate("second") == "first");
		REQUIRE(*strs.allocate("third") == "first");
		REQUIRE(strs.num_available() == 1);
		REQUIRE(strs.num_in_use() == 0);
	}

	SECTION("Reuse") {
		{
			auto a = strs.allocate("first");
			auto b = strs.allocate("second");
			auto c = strs.allocate("third");
			REQUIRE(strs.num_available() == 0);
			REQUIRE(strs.num_in_use() == 3);
		}
		REQUIRE(strs.num_available() == 3);
		REQUIRE(strs.num_in_use() == 0);

		{
			auto a = strs.allocate("aa");
			auto b = strs.allocate("bb");
			auto c = strs.allocate("cc");
			REQUIRE(*a == "first");
			REQUIRE(*b == "second");
			REQUIRE(*c == "third");
		}

		{
			EXPECT_ZERO_ALLOCATIONS(auto a = strs.allocate("aa");
			                        auto b = strs.allocate("bb");
			                        auto c = strs.allocate("cc"););
			EXPECT_ONE_ALLOCATION(
			        auto a = strs.allocate("aa"); auto b = strs.allocate("bb");
			        auto c = strs.allocate("cc"); auto d = strs.allocate("dd"););
		}
		REQUIRE(strs.num_available() == 4);
		REQUIRE(strs.num_in_use() == 0);
		auto a = strs.allocate("aa");
		REQUIRE(strs.num_available() == 3);
		REQUIRE(strs.num_in_use() == 1);
	}
}
