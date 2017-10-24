
#include "catch.hpp"

#include <slot_map.hpp>
#include <atomic>
#include <iostream>


TEST_CASE("slot_map basics", "[slot_map]") {
    constexpr int dataCount(16);

    sc::slot_map<int> map;
    int ids[dataCount];
    int values[dataCount];

    for (int i = 0; i < dataCount / 2; ++i) {
        values[i] = i * 20;
        ids[i] = map.emplace(i * 20);
    }
    for (int i = 0; i < dataCount / 4; ++i) {
        map.erase(ids[i]);
    }
    for (int i = dataCount / 2; i < dataCount; ++i) {
        values[i] = i * 40;
        ids[i] = map.emplace(i * 40);
    }
    for (int i = 0; i < dataCount / 4; ++i) {
        values[i] = i * 10;
        ids[i] = map.emplace(i * 10);
    }

    int sum = 0;
    for (int i = 0; i < dataCount;++i) {
        sum += values[i];
        REQUIRE(values[i] == map[ids[i]]);
    }
    int sumCopy = 0;
    for (auto val : map) sumCopy += val;
    REQUIRE(sum == sumCopy);
}

namespace {
    std::atomic_int ctorsCounter;
    std::atomic_int movesCounter;
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
        explicit Dummy(Dummy const& clone) = delete;
        Dummy& operator=(Dummy const& clone) = delete;
    };
}

TEST_CASE("slot_map ctors, moves & dtors", "[slot_map]") {
    constexpr int dataCount(10);

    sc::slot_map<Dummy> map;
    map.reserve(dataCount);
    int ids[dataCount];

    for (int i = 0; i < dataCount; ++i)
        ids[i] = map.emplace();
    REQUIRE(ctorsCounter == dataCount);

    for (int i = 1; i < dataCount; i += 2)
        map.erase(ids[i]);
    REQUIRE(movesCounter == dataCount / 2);
    REQUIRE(dtorsCounter == dataCount / 2);

    map.reserve(dataCount + 1);
    REQUIRE(ctorsCounter == dataCount);
    REQUIRE(movesCounter == dataCount);
    REQUIRE(dtorsCounter == dataCount);
}
