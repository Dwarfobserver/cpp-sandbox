
#include "catch.hpp"
#include <bytes_units.hpp>
#include <stack_tracker.hpp>

TEST_CASE("stack_array stack checking", "[stack_array]") {
    using namespace sc::bytes_literals;

    sc::initialize_stack_tracker(8_MB);

    int x = 12;
    REQUIRE(sc::is_stack_allocated(&x));

    std::vector<std::unique_ptr<int>> ptrs;
    for (int i = 0; i < 5; ++i) {
        auto& ptr = ptrs.emplace_back(std::make_unique<int>(i));
        REQUIRE(!sc::is_stack_allocated(ptr.get()));
    }
}
