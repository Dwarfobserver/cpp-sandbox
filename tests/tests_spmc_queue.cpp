
#include "catch.hpp"

#include <mpsc_queue.hpp>
#include <thread>


TEST_CASE("mpsc_queue concurrence", "[mpsc_queue]") {
    const int threadCount(std::thread::hardware_concurrency() * 2);
    const int dataCount(threadCount * 1'000);

    sc::mpsc_queue<std::unique_ptr<int>> queue(dataCount / 3);
    std::atomic<int> sum(0);
    std::atomic<int> sumCopy(0);
    std::atomic<int> treated(0);

    std::thread consumer {[&] {
        int sumCopyLocal = 0;
        int treatedLocal = 0;
        while (treatedLocal < dataCount) {
            treatedLocal += queue.consume_all(
                    [&] (auto&& ptr) { sumCopyLocal += *ptr; }
            );
        }
        sumCopy.store(sumCopyLocal);
        treated.store(treatedLocal);
    }};

    std::vector<std::thread> producers;
    for (int t = 0; t < threadCount; ++t) {
        producers.emplace_back([&] {
            int localSum = 0;
            for (int i = 0; i < dataCount / threadCount; ++i) {
                int value = rand() % 10;
                localSum += value;
                queue.push(std::make_unique<int>(value));
            }
            sum.fetch_add(localSum);
        });
    }

    consumer.join();
    for (auto& t : producers) t.join();

    REQUIRE(treated == dataCount);
    REQUIRE(sumCopy == sum.load());
}
