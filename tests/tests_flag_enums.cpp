
#include "catch.hpp"

#include <flag_enums.hpp>


namespace {
    enum class MyFlags : uint64_t {
        None =  0ull,
        All =   static_cast<uint64_t>(~0ull),
        One =   1ull << 0,
        Two =   1ull << 1,
        Three = 1ull << 2,
        Four =  1ull << 3,
    };
}

TEST_CASE("flag_enums basics", "[flag_enums]") {
    using namespace sc::flag_operators;

    auto flags = MyFlags::One | MyFlags::Four;

    auto val = (uint64_t) (MyFlags::One) + (uint64_t) (MyFlags::Four);
    REQUIRE(flags == (MyFlags) val);
}
