
#include "catch.hpp"
#include <stack_allocator.hpp>


TEST_CASE("stack_allocator static", "[stack_allocator]") {
    sc::stack_allocator_resource<false> resource(200);
    sc::stack_allocator<false, int> allocator(resource);

    std::vector<int, sc::stack_allocator<false, int>> vec(allocator);

    vec.reserve(4);
    for (int i = 0; i < 4; ++i) {
        vec.push_back(i);
    }
    auto vec2 = vec;

    REQUIRE(resource.size() == sizeof(int) * 8);
}

TEST_CASE("stack_allocator dynamic", "[stack_allocator]") {
    sc::stack_allocator_resource<true> resource(20);
    sc::stack_allocator<true, int> allocator(resource);

    std::vector<int, sc::stack_allocator<true, int>> vec(allocator);

    vec.reserve(4);
    for (int i = 0; i < 4; ++i) {
        vec.push_back(i);
    }
    auto vec2 = vec;

    REQUIRE(resource.size() == 20 + 4 * sizeof(int));
    REQUIRE(resource.capacity() == 20 * 2);
}
