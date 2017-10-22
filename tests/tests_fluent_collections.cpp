
#include "catch.hpp"

#include <fluent_collections.hpp>
#include <deque>
#include <set>


TEST_CASE("fluent_collections basics", "[fluent_collections]") {
    sc::fluent<std::deque, int> deque;

    for (int i = 0; i < 6; ++i) {
        deque.push_back(i);
    }

    auto result = deque
            .filter([] (auto val) { return val >= 2; })
            .apply( [] (auto& val) { val /= 2; })
            .reduce([] (auto v1, auto v2) { return v1 + v2; });

    REQUIRE(result == 6);
}

TEST_CASE("fluent_collections map", "[fluent_collections]") {
    sc::fluent<std::set, int> set{ 0, 1, 2, 3 };

    auto result = set
            .map(   [] (int val) -> std::string { return std::to_string(val); })
            .reduce([] (auto v1, auto v2) { return v1 + " " + v2; }, "numbers :");

    REQUIRE(result == "numbers : 0 1 2 3");
}
