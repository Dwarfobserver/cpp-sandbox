
#include "catch.hpp"

#include <spsc_queue.hpp>


TEST_CASE("Test correct loop", "[spsc_queue]") {
    constexpr int intCount(10);
    constexpr int queueSize(4);

    sc::spsc_queue<int> queue(queueSize);

    int next = 0;
    int pushes = 0;
    int treated = 0;
    int sum = 0;
    while (next < intCount) {
        queue.push(next++);
        pushes++;
        if (pushes == queueSize) {
            pushes = 0;
            treated += queue.consumeAll([&sum] (int val) {
                sum += val;
            });
        }
    }
    treated += queue.consumeAll([&sum] (int val) {
        sum += val;
    });

    REQUIRE(treated == intCount);
    REQUIRE(sum == (intCount * (intCount - 1)) / 2);
}

static std::atomic_int movesCounter;
static std::atomic_int dtorsCounter;

class Dummy {
public:
    Dummy() = default;
    ~Dummy() {
        dtorsCounter++;
    }
    explicit Dummy(Dummy && old) noexcept {
        movesCounter++;
    }
    Dummy& operator=(Dummy && old) noexcept {
        movesCounter++;
        return *this;
    }
    explicit Dummy(Dummy const& clone) = delete;
    Dummy& operator=(Dummy const& clone) = delete;
};

TEST_CASE("Count moves & destructors", "[spsc_queue]") {
    constexpr int dataCount(3);

    movesCounter = 0;
    dtorsCounter = 0;
    sc::spsc_queue<Dummy> queue(10);
    Dummy dummies[dataCount];

    for (int i = 0; i < dataCount; ++i) {
        queue.emplace(std::move(dummies[i]));
    }
    queue.consumeAll([] (auto&&) {});

    REQUIRE(movesCounter == dataCount);
    REQUIRE(movesCounter == dataCount);
}
