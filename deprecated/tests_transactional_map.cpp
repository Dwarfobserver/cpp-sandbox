
#include "../tests/catch.hpp"

#include "transactional_map.hpp"
#include <thread>

/*
template <class T>
using handle_t = typename sc::transactional_map<T>::handle_t;

TEST_CASE("transactional_map capacity and size", "[transactional_map]") {
    constexpr int capacity(10);

    sc::transactional_map<int> map(capacity);

    for (int i = 0; i < capacity; ++i) {
        map.create();
    }
    try {
        map.create();
        REQUIRE_FALSE("In case of overflow, transactional_map must throw.");
    } catch (std::exception& e) {
        REQUIRE(true);
    }
}

TEST_CASE("transactional_map update & modify", "[transactional_map]") {
    constexpr int dataCount(3);

    sc::transactional_map<int> map(dataCount);
    handle_t<int> handles[dataCount];

    for (auto &handle : handles) {
        REQUIRE(!handle);
        handle = map.create(0);
        REQUIRE(handle);
    }
    for (int j = 0; j < dataCount; ++j) {
        for (int i = j + 1; i < dataCount; ++i) {
            map.modify(handles[i], [] (int& val) {
                ++val;
            });
        }
    }
    for (auto& handle : handles) {
        handle = map.update(handle);
    }

    int val = 0;
    for (auto& handle : handles) {
        REQUIRE(*handle == val);
        ++val;
    }
}

namespace tests_transactional_map {

    struct Counter {
        std::atomic_int moves;
        std::atomic_int copies;
        std::atomic_int dtors;
    };

    class Dummy {
    public:
        Counter* counter;

        Dummy() : counter(nullptr) {}
        explicit Dummy(Counter& counter) : counter(&counter) {}

        ~Dummy() {
            counter->dtors++;
        }
        Dummy(Dummy && old) noexcept : counter(old.counter) {
            counter->moves++;
        }
        Dummy& operator=(Dummy && old) noexcept {
            counter = old.counter;
            counter->moves++;
            return *this;
        }
        Dummy(Dummy const& clone) noexcept : counter(clone.counter) {
            counter->copies++;
        }
        Dummy& operator=(Dummy const& clone) noexcept {
            counter = clone.counter;
            counter->copies++;
            return *this;
        }
    };
}

TEST_CASE("transactional_map copies, moves & deletes", "[transactional_map]") {
    using namespace tests_transactional_map;
    constexpr int dataCount(3);
    constexpr int modifications(2);

    Counter counter {};
    {
        sc::transactional_map<Dummy> map(dataCount); // 1 destructor ??
        handle_t<Dummy> handles[dataCount];
        Dummy dummies[dataCount]; // 3 destructors
        for (auto& dummy : dummies) dummy.counter = &counter;

        for (int i = 0; i < dataCount; ++i) {
            handles[i] = map.create(std::move(dummies[i]));
        } // 3 moves
        for (int i = 0; i < modifications; ++i) {
            handles[i] = map.modify(handles[i], [] (Dummy& dummy) { });
        } // 2 copies
    }
    REQUIRE(counter.moves == dataCount);
    REQUIRE(counter.copies == modifications);
    REQUIRE(counter.dtors == 2 * dataCount + modifications);
}

TEST_CASE("transactional_map clean", "[transactional_map]") {
    using namespace tests_transactional_map;
    constexpr int dataCount(1);

    Counter counter {};
    sc::transactional_map<Dummy> map(dataCount);
    for (int i = 0; i < dataCount - 1; ++i) map.create(counter);

    auto handle = map.create(counter);
    handle = map.modify(handle, [] (Dummy& val) { });
    handle = map.modify(handle, [] (Dummy& val) { });
    handle = map.modify(handle, [] (Dummy& val) { });
    handle = map.modify(handle, [] (Dummy& val) { });

    map.clean();

    REQUIRE(counter.dtors == 2);
}
*/