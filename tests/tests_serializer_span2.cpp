
#include "catch.hpp"
#include <serializer_span2.hpp>
#include <iostream>


namespace {
    struct point {
        uint8_t x;
        uint8_t y;
    };
}

template<class Span>
Span &operator&(Span &span, point &p) {
    return span & p.x & p.y;
}

TEST_CASE("serializer_span2 basics", "[serializer_span2]") {
    std::byte storage[20];

    sc::binary_ospan ospan{storage};
    point p{2, 3};
    ospan << p;
    REQUIRE(ospan.begin - storage == sc::serialized_size<point>());

    sc::binary_ispan ispan{storage};
    p = point{};
    ispan >> p;
    REQUIRE(p.x == 2);
    REQUIRE(p.y == 3);
}

