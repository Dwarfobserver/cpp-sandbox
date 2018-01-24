
#include "catch.hpp"
#include <serializer_span2.hpp>


namespace {
    struct point3D {
        float x, y, z;
    };
}
namespace sc {
    template<> constexpr bool is_trivially_serializable<point3D> = true;
}

TEST_CASE("serializer_span2 trivial types", "[serializer_span2]") {
    std::byte storage[20];
    sc::binary_span span{storage};
    span << 1.f << 2.f << 3.f;

    static_assert(sc::serialized_size<float>() == 4);
    static_assert(sc::serialized_size<point3D>() == sc::serialized_size<float>() * 3);
    REQUIRE(span.begin == storage + sc::serialized_size<point3D>());

    point3D p {};
    span.begin = storage;
    span >> p;

    REQUIRE(p.x == 1.f);
    REQUIRE(p.y == 2.f);
    REQUIRE(p.z == 3.f);
}

namespace {
    struct point {
        uint8_t x;
        uint8_t y;
    };
    template<class Span>
    auto &operator&(Span &span, point &p) {
        return span & p.x & p.y;
    }

    struct triangle {
        point p1;
        point p2;
        point p3;
    };
    template<class Span>
    auto &operator&(Span &span, triangle &t) {
        return span & t.p1 & t.p2 & t.p3;
    }
}

TEST_CASE("serializer_span2 basic type", "[serializer_span2]") {
    std::byte storage[20];

    sc::binary_span span{storage};
    triangle t{{1, 2}, {3, 4}, {5, 6}};
    span << t;
    REQUIRE(span.begin - storage == sc::serialized_size<triangle>());

    span.begin = storage;
    t = triangle{};
    span >> t;
    REQUIRE(t.p1.x == 1);
    REQUIRE(t.p2.y == 4);
}

TEST_CASE("serializer_span2 continuous storages", "[serializer_span2]") {
    std::byte storage[20];

    sc::binary_span span{storage};
    std::string str{"Hello world !"};
    span << str;
    REQUIRE(span.begin - storage == sc::serialized_size(str));

    span.begin = storage;
    str.clear();
    span >> str;
    REQUIRE(str == "Hello World !");
}
