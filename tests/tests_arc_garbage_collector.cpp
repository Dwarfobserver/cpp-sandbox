
#include "catch.hpp"
#include <arc_garbage_collector.hpp>


TEST_CASE("arc_garbage_collector basics", "[arc_garbage_collector]") {
    sc::arc_garbage_collector gc;
    {
        auto pInt = gc.factory<int>().make(42);
        auto pIntCopy = pInt;
        REQUIRE(*pInt == 42);

        gc.collect();
        REQUIRE(gc.references_count() == 1);
    }
    gc.collect();
    REQUIRE(gc.references_count() == 0);
}

