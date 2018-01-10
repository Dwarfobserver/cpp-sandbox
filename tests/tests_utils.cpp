
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

TEST_CASE("utils aligned_allocator", "[utils]") {
    using alloc512_t = sc::aligned_allocator<int, std::allocator, 512>;

    int* aligned512 = alloc512_t().allocate(10);
    alloc512_t().deallocate(aligned512, 10);

    REQUIRE(reinterpret_cast<intptr_t>(aligned512) % 512 == 0);
}
