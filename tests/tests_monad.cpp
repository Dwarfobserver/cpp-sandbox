
#include "catch.hpp"
#include <monad.hpp>
#include <numeric>


TEST_CASE("optionals monad", "[monad]") {
    using namespace sc::monad_operator;

    static_assert(!sc::is_monad<std::allocator>);
    static_assert(sc::is_monad<std::optional>);
    static_assert(!sc::detail::is_joinable_monad<std::optional, std::optional<int>>);
    static_assert(sc::detail::is_joinable_monad<std::optional, std::optional<std::optional<int>>>);

    auto to_str = [] (int x)
        { return std::optional{ std::to_string(x) }; };

    auto length = [] (std::string const& str)
        { return str.size(); };

    std::optional<int> val;
    auto res = val | to_str | length | [] (auto) { throw std::runtime_error{"derp"}; return 0; };
    REQUIRE(!res);

    val = 1234;
    REQUIRE(*(val | to_str | length) == 4);
}

TEST_CASE("optionals containers", "[monad]") {
    using namespace sc::monad_operator;

    static_assert(sc::is_iterator<std::vector<float>::iterator>);
    static_assert(sc::is_iterable<std::vector<float>>);
    static_assert(sc::can_emplace_in<std::vector<float>>);

    auto to_str = [] (auto x) {
        auto str = std::to_string(x);
        return std::vector<char>(str.begin(), str.end());
    };

    auto to_int = [] (char c) -> int { return c - '0'; };

    std::vector<int> vals { 15, 24, 7 };
    auto res = vals | to_str | to_int;
    REQUIRE(std::accumulate(res.begin(), res.end(), 0) == 1+5 + 2+4 + 7);

    vals.clear();
    res = vals | to_str | to_int | [] (auto) { return 42; };
    REQUIRE(std::accumulate(res.begin(), res.end(), 0) == 0);
}
