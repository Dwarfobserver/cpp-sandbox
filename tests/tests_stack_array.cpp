
#include "catch.hpp"
#include <stack_array.hpp>
#include <numeric>

TEST_CASE("stack_array constructors", "[stack_array]") {
    sc::stack_array<int> zeroes(3);
    REQUIRE(std::accumulate(zeroes.begin(), zeroes.end(), 0) == 0);

    sc::stack_array<int> fives(4, 5);
    REQUIRE(std::accumulate(fives.begin(), fives.end(), 0) == fives.size() * 5);

    sc::stack_array<int> ints{ 2, 6, 7, 9 };
    REQUIRE(std::accumulate(ints.begin(), ints.end(), 0) == 2 + 6 + 7 + 9);

    sc::stack_array<int> ints_copy(ints.begin(), ints.end());
    REQUIRE(std::accumulate(ints_copy.begin(), ints_copy.end(), 0) == 2 + 6 + 7 + 9);
}
