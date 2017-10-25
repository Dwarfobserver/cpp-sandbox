
#include "catch.hpp"

#include <block_allocator.hpp>


TEST_CASE("block_allocator basics", "[block_allocator]") {
    using alloc_t = sc::block_allocator<int, std::allocator, 500, 64>;

    int* pValues[2];

    pValues[0] = alloc_t().allocate(1);
    *pValues[0] = 10;
    pValues[1] = alloc_t().allocate(1);
    *pValues[1] = 20;
    alloc_t().deallocate(pValues[0], 1);
    pValues[0] = alloc_t().allocate(1);
    *pValues[0] = 30;

    REQUIRE(*pValues[0] == 30);
    REQUIRE(reinterpret_cast<intptr_t>(pValues[0]) % 64 == 0);
    REQUIRE(*pValues[1] == 20);
    REQUIRE(reinterpret_cast<intptr_t>(pValues[1]) % 64 == 0);
}
