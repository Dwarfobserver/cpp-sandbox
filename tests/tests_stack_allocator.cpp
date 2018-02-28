
#include "catch.hpp"
#include <stack_allocator.hpp>


TEST_CASE("stack_allocator basics", "[stack_allocator]") {
    using vector_t = std::vector<int, sc::stack_allocator<int>>;

    sc::stack_resource resource(200);
    REQUIRE(resource.capacity() == 200);

    resource.push(7);
    {
        sc::stack_guard lock{ resource };
        vector_t vec{ lock.get_allocator<int>() };
        vec.reserve(10);
        {
            bool caught = false;
            try {
                vec.reserve(resource.capacity() / sizeof(int));
            }
            catch(...) {
                caught = true;
            }
            REQUIRE(caught);
        }
        REQUIRE(resource.size() == 7 + 10 * sizeof(int));
    }
    REQUIRE(resource.size() == 7);

    resource.pop(7);
    REQUIRE(resource.size() == 0);
}
