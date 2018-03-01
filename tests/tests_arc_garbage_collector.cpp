
#include "catch.hpp"
#include <arc_garbage_collector.hpp>


TEST_CASE("arc_garbage_collector reference count", "[arc_garbage_collector]") {

    int dummies_dead = 0;

    struct Dummy {
        int& flag_;
        Dummy(int& flag) : flag_(flag) {}
        ~Dummy() noexcept { flag_ += 1; }
    };

    sc::arc_garbage_collector gc;
    {
        std::vector<sc::gc_ptr<Dummy>> pDummies;
        for (int i = 0; i < 30; ++i) {
            auto ptr = gc.factory<Dummy>().make(dummies_dead);
            if (i % 3) pDummies.push_back(std::move(ptr));
        }
        REQUIRE(dummies_dead == 0);

        // Last object created : i = 29, i % 3 != 0 so we don't miss an object to delete
        // (async_collect() will not test the last object created)
        gc.async_collect();
        REQUIRE(dummies_dead == 10);
    }
    gc.collect();
    REQUIRE(dummies_dead == 30);
}

