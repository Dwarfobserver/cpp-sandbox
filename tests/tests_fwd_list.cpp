
#include "catch.hpp"

#include <fwd_list.hpp>
#include <thread>


namespace tests_fwd_list {
    static std::atomic_int movesCounter;
    static std::atomic_int dtorsCounter;

    class Dummy {
    public:
        Dummy() = default;
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

TEST_CASE("Check moves & destructors", "[fwd_list]") {
    using namespace tests_fwd_list;
    constexpr int countData(4);

    movesCounter.store(0);
    dtorsCounter.store(0);
    {
        sc::fwd_list<Dummy> list;

        for (int i = 0; i < countData; ++i) {
            list.push_front(Dummy{});
        }
    }

    REQUIRE(movesCounter.load() == countData);
    REQUIRE(dtorsCounter.load() == countData);
}

TEST_CASE("Check iterators", "[fwd_list]") {
    constexpr int countData(3);
    sc::fwd_list<int> list;

    REQUIRE(list.begin() == list.cend());

    for (int i = 0; i < countData; ++i) {
        list.push_front(42);
    }
    int i = 0;
    for (int& val : list) {
        val = i++;
    }
    i = 0;
    auto it = list.cbegin();
    while (it != list.cend()) {
        REQUIRE(*(it++) == i++);
    }
}

