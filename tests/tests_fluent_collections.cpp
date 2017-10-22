
#include "catch.hpp"

#include <fluent_collections.hpp>
#include <deque>
#include <set>


TEST_CASE("fluent_collections basics", "[fluent_collections]") {
    const std::deque<int> original {0, 1, 2, 3, 4, 5};
    const sc::fluent<std::deque, int> deque(original);

    const auto result = deque
            .filter([] (auto val) { return val >= 2; })
            .apply( [] (auto& val) { val /= 2; })
            .reduce([] (auto v1, auto v2) { return v1 + v2; });

    REQUIRE(result == 6);
}

TEST_CASE("fluent_collections map", "[fluent_collections]") {
    const sc::fluent<std::set, int> set{ 0, 1, 2, 3 };

    const auto result = set
            .map(   [] (auto val) { return std::to_string(val); })
            .reduce([] (auto v1, auto v2) { return v1 + " " + v2; }, "numbers :");

    REQUIRE(result == "numbers : 0 1 2 3");
}
