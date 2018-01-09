
#include "catch.hpp"
#include <monad.hpp>


TEST_CASE("optionals monad", "[monad]") {
    using namespace sc::monad_operator;

    auto to_str = [] (int x)
        { return std::optional{ std::to_string(x) }; };

    auto length = [] (std::string const& str)
        { return std::optional{ str.size() }; };

    std::optional<int> val;
    auto res = val | to_str | length;
    REQUIRE(!res);

    val = 1234;
    REQUIRE(*(val | to_str | length) == 4);
}
