
#include "catch.hpp"

#include <utils.hpp>


TEST_CASE("utils defer", "[utils]") {
    int i = 0;
    {
        sc::defer setter{[&] () noexcept { i = 42; }};
        i = 13;
    }
    REQUIRE(i == 42);
}


template <class Concept, class T>
auto operation(Concept& c, T const& data)
-> Concept& {
    return ++c;
};

template <class Concept>
auto operation(Concept& c, int data)
-> sc::return_if_t<Concept, std::is_same_v<Concept, int>>& {
    c+= data;
    return c;
};

TEST_CASE("utils return_if_t", "[utils]") {
    float f = 0;
    REQUIRE(operation(f, 42) == 1);
    int value = 0;
    operation(value, 2);
    REQUIRE(value == 2);
}

TEST_CASE("utils optional_monad", "[utils]") {
    using namespace sc::optional_monad;

    auto computed = std::optional<int>(5)
        | [] (auto val) { return val * 3; }
        | [] (auto val) -> std::string { return std::to_string(val); };

    REQUIRE(*computed == "15");

    bool mustStayTrue = true;
    auto empty = std::optional<int>(3)
        | [] (int) { return std::optional<int>(); }
        | [&] (int) { mustStayTrue = false; return false; };

    REQUIRE(!empty);
    REQUIRE(mustStayTrue);
}

TEST_CASE("utils aligned_allocator", "[utils]") {
    using alloc512_t = sc::aligned_allocator<int, std::allocator, 512>;

    int* aligned512 = alloc512_t().allocate(10);
    alloc512_t().deallocate(aligned512, 10);

    REQUIRE(reinterpret_cast<intptr_t>(aligned512) % 512 == 0);
}
