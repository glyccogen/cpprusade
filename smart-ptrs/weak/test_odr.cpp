#include "shared.h"
#include "weak.h"

#include "../../.for_tests/catch.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("Just checking for ODR issues") {
	auto sp = makeShared<int>(42);
	WeakPtr<int> wp(sp);
	REQUIRE(!wp.expired());
	auto sp2 = wp.lock();
	REQUIRE(*sp2 == 42);
}
