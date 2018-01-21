
#include "catch.hpp"
#include <movable_function.hpp>
#include <functional>

namespace {
    void increment(int& val) { ++val; }
}

TEST_CASE("movable_function type erasure", "[movable_function]") {
    sc::movable_function<void(int&)> inc_f = increment;
    int val = 2;
    inc_f(val);
    REQUIRE(val == 3);

    auto pName = std::make_unique<std::string>("yolandis");
    auto name = *pName;
    sc::movable_function<std::string()> make_name_f(
            [pName = std::move(pName)] {
        return *pName;
    });
    REQUIRE(make_name_f() == name);
}

namespace {
    struct Dummy {
        int moveCount;
        int operator()() {
            return moveCount;
        }

        Dummy() : moveCount(0) {}

        Dummy(Dummy const&) = delete;
        Dummy(Dummy&& d) noexcept :
                moveCount(d.moveCount + 1) {
            d.moveCount = 0;
        }

        Dummy& operator=(Dummy const&) = delete;
        Dummy& operator=(Dummy&& d) noexcept {
            moveCount = d.moveCount + 1;
            d.moveCount = 0;
        }
    };
}

TEST_CASE("movable_function move operations", "[movable_function]") {
    sc::movable_function<int()> wrapper;
    wrapper = {Dummy{}};

    sc::movable_function<int()> wrapper2;
    wrapper2 = std::move(wrapper);

    REQUIRE(wrapper2() == 1);
}
