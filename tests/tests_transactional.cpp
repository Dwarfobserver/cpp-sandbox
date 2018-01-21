
#include "catch.hpp"

#include <transactional.hpp>


namespace {
    std::atomic_int ctorsCounter;
    std::atomic_int movesCounter;
    std::atomic_int copiesCounter;
    std::atomic_int dtorsCounter;

    class Dummy {
    public:
        Dummy() {
            ctorsCounter++;
        }
        ~Dummy() {
            dtorsCounter++;
        }
        Dummy(Dummy && old) noexcept {
            movesCounter++;
        }
        Dummy& operator=(Dummy && old) noexcept {
            movesCounter++;
            return *this;
        }
        Dummy(Dummy const& clone) {
            copiesCounter++;
        }
        Dummy& operator=(Dummy const& clone) {
            copiesCounter++;
            return *this;
        }
    };
}

TEST_CASE("transactional ctors, moves & copies count", "[transactional]") {
    sc::transactional<Dummy> dummy;
    {
        sc::transactional<Dummy>::handle_t handle;
        REQUIRE(dummy.empty());
        {
            handle = dummy.create();
            auto h2 = handle;
            dummy.modify(h2, [](Dummy &val) noexcept {});
            REQUIRE(ctorsCounter == 1);
            REQUIRE(movesCounter == 0);
            REQUIRE(copiesCounter == 1);
            REQUIRE(dtorsCounter == 0);
        }
        dummy.update(handle);
        dummy.clear();
        REQUIRE(dtorsCounter == 1);
    }
    dummy.clear();
    REQUIRE(dtorsCounter == 2);
}


