
#include "catch.hpp"
#include <make_string.hpp>
#include <vector>
#include <map>

namespace {
    struct stringable_t {};
    std::string to_string(stringable_t const&) {
        return "stringable";
    }
    struct streamable_t {};
    std::ostringstream& operator<<(std::ostringstream& oss, streamable_t const&) {
        oss << "streamable";
        return oss;
    }

}

TEST_CASE("make_string all categories", "[make_string]") {
    REQUIRE(sc::make_string("derp") == "derp");
    REQUIRE(sc::make_string(2) == "2");
    REQUIRE(sc::make_string(stringable_t{}) == "stringable");
    REQUIRE(sc::make_string(streamable_t{}) == "streamable");

    static_assert(std::is_same_v<sc::traits::iterator_value_of<std::vector<int>>, int>);
    REQUIRE(sc::make_string(std::vector<int>{ 0, 1, 2 }) == "[ 0, 1, 2 ]");

    REQUIRE(sc::make_string(std::make_tuple(42, "hey", true)) == "{ 42, hey, true }");
    REQUIRE(sc::make_string(std::make_pair('x', "lapin")) == "{ x, lapin }");

    std::map<std::vector<int>, char const*> data {
        {{-12, 4}, "sire"},
        {{8, 0, 73}, "denez"}
    };
    REQUIRE(sc::make_string(data) == "[ { [ -12, 4 ], sire }, { [ 8, 0, 73 ], denez } ]");
}
