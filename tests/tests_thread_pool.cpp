
#include "catch.hpp"
#include <thread_pool.hpp>
#include <iostream>

TEST_CASE("thread_pool basics", "[thread_pool]") {
    sc::thread_pool threadPool{3};

    std::vector<std::future<int>> results;
    for (int i = 1; i <= 50; ++i) {
        results.push_back(threadPool.execute([i] {
            return i;
        }));
    }
    int sum = 0;
    for (auto &result : results) {
        sum += result.get();
    }
    REQUIRE(sum == 50 * 51 / 2);
}
