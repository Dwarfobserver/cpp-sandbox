
#include "catch.hpp"
#include <serializer_span.hpp>


namespace {
    struct point3D {
        float x, y, z;
    };
}
namespace sc {
    template<> constexpr bool is_trivially_serializable<point3D> = true;
}

TEST_CASE("serializer_span trivial types", "[serializer_span]") {
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
        point p1, p2, p3, padding;
    };
    template<class Span>
    auto &operator&(Span &span, triangle &t) {
        return span & t.p1 & t.p2 & t.p3;
    }
}

TEST_CASE("serializer_span basic type", "[serializer_span]") {
    std::byte storage[20];

    sc::binary_span span{storage};
    triangle t{{1, 2}, {3, 4}, {5, 6}};
    span << t;
    static_assert(sc::serialized_size<triangle>() ==
                  sc::serialized_size<point>() * 3);
    REQUIRE(span.begin - storage == sc::serialized_size<triangle>());

    span.begin = storage;
    t = {};
    span >> t;
    REQUIRE(t.p1.x == 1);
    REQUIRE(t.p2.y == 4);
}

TEST_CASE("serializer_span pair & tuple types", "[serializer_span]") {
    using pair_t = std::pair<uint8_t, int16_t>;
    using tuple_t = std::tuple<int32_t, float, uint32_t>;

    std::byte storage[20];
    sc::binary_span span{storage};

    auto pair  = pair_t(20, 30);
    auto tuple = tuple_t(1, 1.5f, 2);

    span << pair << tuple;
    REQUIRE(span.begin == storage + sc::serialized_size<pair_t>() + sc::serialized_size<tuple_t>());

    pair  = {};
    tuple = {};
    span.begin = storage;
    span >> pair >> tuple;

    REQUIRE(pair.second == 30);
    REQUIRE(std::get<int32_t>(tuple) == 1);
    REQUIRE(std::get<float>(tuple) == 1.5f);
}
/*
TEST_CASE("serializer_span iterables", "[serializer_span]") {
    static_assert(sc::is_iterable<std::string>);

    std::byte storage[20];
    sc::binary_span span{storage};

    auto str = std::string{"Hello World !"};

    span << str;
    REQUIRE(span.begin == storage + sc::serialized_size(str));

    str = {};
    span.begin = storage;
    span >> str;

    REQUIRE(str == "Hello World !");
}*/
