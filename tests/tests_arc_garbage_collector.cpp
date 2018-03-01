
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
        REQUIRE(gc.references_count() == 30);
        REQUIRE(dummies_dead == 0);

        gc.collect();
        REQUIRE(gc.references_count() == 20);
        REQUIRE(dummies_dead == 10);
    }
    gc.collect();
    REQUIRE(gc.references_count() == 0);
    REQUIRE(dummies_dead == 30);
}

