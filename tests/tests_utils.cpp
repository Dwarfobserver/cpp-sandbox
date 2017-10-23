
#include "catch.hpp"

#include <utils.hpp>


TEST_CASE("utils defer", "[utils]") {
    int i = 0;
    {
        sc::defer setter{[&] () noexcept { i = 42; }};
        i = 13;
    }
    REQUIRE(i == 42);
}

TEST_CASE("utils optional_monad", "[utils]") {
    using namespace sc::optional_monad;

    auto computed = std::optional<int>(5)
        | [] (auto val) { return val * 3; }
        | [] (auto val) -> std::string { return std::to_string(val); };

    REQUIRE(*computed == "15");

    bool mustStayTrue = true;
    auto empty = std::optional<int>(3)
        | [] (int) { return std::optional<int>(); }
        | [&] (int) { mustStayTrue = false; return false; };

    REQUIRE(!empty);
    REQUIRE(mustStayTrue);
}

TEST_CASE("utils aligned_allocator", "[utils]") {
    using alloc512_t = sc::aligned_allocator<int, std::allocator, 512>;

    int* aligned512 = alloc512_t().allocate(1);
    alloc512_t().deallocate(aligned512, 1);

    REQUIRE(reinterpret_cast<intptr_t>(aligned512) % 512 == 0);
}
