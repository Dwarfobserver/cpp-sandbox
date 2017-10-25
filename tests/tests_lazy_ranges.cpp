
#include "catch.hpp"

#include <lazy_ranges.hpp>


TEST_CASE("lazy_ranges basics", "[lazy_ranges]") {
    std::vector<int> vec;
    for (int i = 0; i < 4; ++i) {
        vec.push_back(i);
    }

    std::vector<int> vecCopy;
    sc::lazy_range(vec.begin(), vec.end())
        .map([] {})
        .eval(std::back_inserter(vecCopy));

    REQUIRE(vec[2] == vecCopy[2]);
}
