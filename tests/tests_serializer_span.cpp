
#include "catch.hpp"

#include <serializer_span.hpp>
#include <utils.hpp>


TEST_CASE("serializer_span primitive types", "[serializer_span]") {
    using namespace sc::serializer;

    std::byte storage1[20] {};
    std::byte storage2[20] {};

    binary_ospan oSpan{storage1, sizeof(int) * 3};
    oSpan << 1 << 2 << 3;
    REQUIRE(oSpan.size == 0);

    binary_ispan iSpan{storage1, 20};
    binary_iospan ioSpan{storage2, 20};
    for (int i = 0; i < 3; ++i) {
        int val;
        iSpan >> val;
        ioSpan << val;
    }
    ioSpan = {storage2, 100};
    for (int i = 1; i <= 3; ++i) {
        int val;
        ioSpan >> val;
        REQUIRE(val == i);
    }
}

TEST_CASE("serializer_span overloads", "[serializer_span]") {
    using namespace sc::serializer;

    std::vector<int> ints {1, 2, 3, 4, 5};

    std::byte storage[50];
    binary_iospan span {storage, 50};
    span << ints;

    std::vector<int> intsCopy;
    span = {storage, 50};
    span >> intsCopy;

    REQUIRE(ints == intsCopy);
}
