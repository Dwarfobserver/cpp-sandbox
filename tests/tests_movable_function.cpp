
#include "catch.hpp"
#include <movable_function.hpp>
#include <functional>

namespace {
    void increment(int& val) { ++val; }
}

TEST_CASE("movable_function type erasure", "[movable_function]") {
    sc::movable_function<void(int&)> inc_f(increment);
    int val = 2;
    inc_f(val);
    REQUIRE(val == 3);

    std::string name{"yolandis"};
    sc::movable_function<std::string()> make_name_f([name] {
        return name;
    });
    REQUIRE(make_name_f() == name);
}

TEST_CASE("movable_function move operations", "[movable_function]") {
    auto pVal = std::make_unique<int>(42);
    sc::movable_function<int()> gen_int_f([pVal = std::move(pVal)] {
        return (*pVal)++;
    });
    REQUIRE(gen_int_f.is_valid());

    sc::movable_function<int()> f_copy;
    sc::swap(gen_int_f, f_copy);
    REQUIRE(!gen_int_f);
    REQUIRE(f_copy() == 42);
    REQUIRE(f_copy() == 43);
}
