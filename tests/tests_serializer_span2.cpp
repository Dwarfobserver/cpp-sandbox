
#include "catch.hpp"
#include <serializer_span2.hpp>


namespace {
    struct point {
        uint8_t x;
        uint8_t y;
    };

    template<class Span>
    constexpr auto& operator&(Span& span, point& p) {
        return span & p.x & p.y;
    }

    struct triangle {
        point p1;
        point p2;
        point p3;
    };

    template<class Span>
    constexpr auto& operator&(Span& span, triangle& t) {
        return span & t.p1 & t.p2 & t.p3;
    }

}

TEST_CASE("serializer_span2 basics", "[serializer_span2]") {
    std::byte storage[20];

    sc::binary_ospan ospan{storage};
    //point p{2, 3};
    ospan << 32;/*
    static_assert(sc::serialized_size<point>() == 2 * sizeof(uint8_t));
    REQUIRE(ospan.begin - storage == sc::serialized_size<point>());

    sc::binary_ispan ispan{storage};
    p = point{};
    ispan >> p;
    REQUIRE(p.x == 2);
    REQUIRE(p.y == 3);*/
}
/*
TEST_CASE("serializer_span2 recurrence", "[serializer_span2]") {
    std::byte storage[20];

    sc::binary_ospan ospan{storage};
    triangle t{{1, 2}, {3, 4}, {5, 6}};
    ospan << t;
    static_assert(sc::serialized_size<triangle>() == 6 * sizeof(uint8_t));
    REQUIRE(ospan.begin - storage == sc::serialized_size<triangle>());

    sc::binary_ispan ispan{storage};
    t = triangle{};
    ispan >> t;
    REQUIRE(t.p1.x == 1);
    REQUIRE(t.p2.y == 4);
}
*/
