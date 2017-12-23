
#include "catch.hpp"

#include <serializer_span.hpp>
#include <functional>
#include <condition_variable>


TEST_CASE("serializer_span primitive types", "[serializer_span]") {
    using namespace sc::serializer;

    std::byte storage1[20] {};
    std::byte storage2[20] {};

    binary_ospan<policy::throwing> oSpan{storage1, storage1 + sizeof(int) * 3};
    oSpan << 1 << 2 << 3;
    REQUIRE(oSpan.size() == 0);

    binary_ispan<policy::throwing> iSpan{storage1, storage1 + 20};
    binary_iospan<policy::throwing> ioSpan{storage2, storage1 + 20};
    for (int i = 0; i < 3; ++i) {
        int val;
        iSpan >> val;
        ioSpan << val; // Catch bug in release mode
    }
    ioSpan = {storage2, storage1 + 100};
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
    binary_iospan<policy::throwing> span {storage, storage + 50};
    span << ints;

    std::vector<int> intsCopy;
    span = {storage, storage + 50};
    span >> intsCopy;

    REQUIRE(ints == intsCopy);

    std::string str = "Hello world !";
    span = {storage, storage + 50};
    span << str;

    std::string strCopy;
    span = {storage, storage + 50};
    span >> strCopy;

    REQUIRE(str == strCopy);
}

TEST_CASE("serializer_span policies", "[serializer_span]") {
    using namespace sc::serializer;

    std::byte storage[50];
    binary_iospan<policy::unsafe> unsafeSpan {storage, storage + 7};
    unsafeSpan << 1 << 2;
    REQUIRE(unsafeSpan.begin > unsafeSpan.end);

    binary_iospan<policy::safe> safeSpan {storage, storage + 7};
    safeSpan << 1 << 2;
    REQUIRE(safeSpan.end == nullptr);

    bool error = false;
    binary_iospan<policy::throwing> throwingSpan {storage, storage + 7};
    try {
        throwingSpan << 1 << 2;
    }
    catch (overflow_exception& e) {
        error = true;
    }
    REQUIRE(error);
}
