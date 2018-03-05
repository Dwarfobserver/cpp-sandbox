
#include "catch.hpp"
#include <compiler_hints.hpp>
#include <random>


NO_INLINE int get_currency_value(int type) {
    ASSERT(type >= 1 && type <= 4, "Wrong currency type value");
    switch (type) {
        case 1: return 1;
        case 2: return 2;
        case 3: return 5;
        case 4: return 10;
        default: UNREACHABLE();
    }
}

FORCE_INLINE int add_and_increment(int* RESTRICT i1, int* RESTRICT i2) {
    ++(*i1);
    ++(*i2);
    return *i1 + *i2;
}

TEST_CASE("compiler_hints demo", "[compiler_hints]") {
    REQUIRE(get_currency_value(4) == 10);

    int int1 = 42;
    int int2 = 13;
    REQUIRE(add_and_increment(&int1, &int2) == 43 + 14);

    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<int> uni(1, 100);

    if UNLIKELY(uni(rng) == 100) { "loto winner"; }
}
