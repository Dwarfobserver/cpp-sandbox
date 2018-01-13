
#include "catch.hpp"
#include <type_traits.hpp>
#include <set>
#include <forward_list>


TEST_CASE("type_traits is_iterator", "[type_traits]") {
    REQUIRE(!sc::is_iterator<int>);
    REQUIRE( sc::is_iterator<int*>);
    REQUIRE( sc::is_iterator<typename std::vector<int>::iterator>);
}

TEST_CASE("type_traits is_iterable", "[type_traits]") {
    REQUIRE(!sc::is_iterable<int>);
    REQUIRE(!sc::is_iterable<int*>);
    REQUIRE( sc::is_iterable<std::vector<int>>);
}

TEST_CASE("type_traits can_emplace_in", "[type_traits]") {
    REQUIRE(!sc::can_emplace_in<int>);
    REQUIRE(!sc::can_emplace_in<int*>);
    REQUIRE( sc::can_emplace_in<std::set<int>>);
    REQUIRE( sc::can_emplace_in<std::vector<int>>);
    REQUIRE(!sc::can_emplace_in<std::array<int, 4>>);
    REQUIRE( sc::can_emplace_in<std::forward_list<int>>);
}

namespace {
    struct fat_t {
        char burgers[128];
    };
    struct fast_copy_t {
        void* p1;
        void* p2;
    };
    struct slow_copy_t {
        int i;
        slow_copy_t(slow_copy_t const&) {
            i = 42'000;
        }
    };
}

TEST_CASE("type_traits efficient_argument_t", "[type_traits]") {
    // Selection on size
    REQUIRE(!std::is_reference_v<sc::efficient_argument_t<int>>);
    REQUIRE(!std::is_reference_v<sc::efficient_argument_t<fast_copy_t>>);
    REQUIRE( std::is_reference_v<sc::efficient_argument_t<fat_t>>);

    // Selection on ctor
    REQUIRE( std::is_reference_v<sc::efficient_argument_t<slow_copy_t>>);
}
