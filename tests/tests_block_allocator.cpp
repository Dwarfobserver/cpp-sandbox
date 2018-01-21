
#include "catch.hpp"

#include <block_allocator.hpp>


TEST_CASE("block_allocator static", "[block_allocator]") {
    sc::block_allocator_resource<false, int> resource(3);
    sc::block_allocator<false, int> allocator(resource);

    auto i1 = allocator.allocate(1);
    auto i2 = allocator.allocate(1);

    REQUIRE(resource.size() == 2);

    allocator.deallocate(i2, 1);
    i2 = allocator.allocate(1);
    auto i3 = allocator.allocate(1);

    REQUIRE(resource.size() == resource.capacity());
}
