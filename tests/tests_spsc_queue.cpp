
#include "catch.hpp"

#include <spsc_queue.hpp>
#include <thread>


TEST_CASE("spsc_queue capacity and size", "[spsc_queue]") {
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
            treated += queue.consume_all([&sum](int val) {
                sum += val;
            });
        }
    }
    treated += queue.consume_all([&sum](int val) {
        sum += val;
    });

    REQUIRE(treated == intCount);
    REQUIRE(sum == (intCount * (intCount - 1)) / 2);
}

namespace {
    std::atomic_int movesCounter;
    std::atomic_int dtorsCounter;

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

TEST_CASE("spsc_queue moves & destructors", "[spsc_queue]") {
    constexpr int dataCount(3);

    movesCounter = 0;
    dtorsCounter = 0;
    {
        sc::spsc_queue<Dummy> queue(dataCount);
        Dummy dummies[dataCount];
        for (auto &val : dummies) {
            queue.push(std::move(val));
        }
    }

    REQUIRE(movesCounter == dataCount);
    REQUIRE(dtorsCounter == dataCount * 2);
}

namespace {
    struct alignas(32) Aligned32 {};
    struct alignas(64) Aligned64 {};
}

TEST_CASE("spsc_queue data alignment", "[spsc_queue]") {
    constexpr int dataCount(3);

    sc::spsc_queue<Aligned32> queue32(dataCount);
    sc::spsc_queue<Aligned64> queue64(dataCount);

    for (int i = 0; i < dataCount; ++i) {
        queue32.push(Aligned32{});
        queue64.push(Aligned64{});
    }

    auto test = [&] (auto&& val, int align) {
        auto ptr_val = reinterpret_cast<intptr_t>(&val);
        REQUIRE(ptr_val % align == 0);
    };
    queue32.consume_all([&](Aligned32 &&val) { test(val, 32); });
    queue64.consume_all([&](Aligned64 &&val) { test(val, 64); });
}

TEST_CASE("spsc_queue concurrence", "[spsc_queue]") {
    constexpr int dataCount(10'000);
    using data_t = std::array<int, 32>;

    // To test better atomic ops, should use N threads > std::thread::hardware_concurrency() (some go to sleep)

    sc::spsc_queue<data_t> queue(dataCount / 2);
    std::atomic_int sum(0);
    std::atomic_int sumCopy(0);
    auto producer = [&] {
        int localSum = 0;
        data_t buf{};
        for (int i = 0; i < dataCount; ++i) {
            for (int &val : buf) {
                val = rand() % 1'000; // NOLINT
                localSum += val;
            }
            queue.push(buf);
        }
        sum.store(localSum, std::memory_order_release);
    };
    auto consumer = [&] {
        int localSum = 0;
        int treated = 0;
        while (treated < dataCount) {
            treated += queue.consume_all([&] (data_t&& data) {
                for (int val : data) {
                    localSum += val;
                }
            });
        }
        sumCopy.store(localSum, std::memory_order_release);
    };
    std::thread tProducer {[&] { producer(); }};
    consumer();
    tProducer.join();

    REQUIRE(sum.load() == sumCopy.load());
}
