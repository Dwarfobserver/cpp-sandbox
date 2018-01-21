
#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <lazy_ranges.hpp>
#include <fluent_collections.hpp>
#include <pod_vector.hpp>

#include <chrono>
#include <functional>
#include <mutex>
#include <thread>
#include <queue>
#include <mpsc_queue.hpp>


namespace {

    auto mesure(std::function<void()> const& f) {
        using namespace std::chrono;
        auto tBegin = high_resolution_clock::now();
        f();
        auto tEnd = high_resolution_clock::now();
        return duration_cast<microseconds>(tEnd - tBegin).count();
    };

    auto mesure_tasks(std::vector<std::function<void()>> const& tasks, int passes = 10) {
        std::vector<long long> mesures(tasks.size());

        for (int pass = 0; pass < passes; ++pass) {
            for (int task = 0; task < tasks.size(); ++task) {
                mesures[task] += mesure(tasks[task]);
            }
        }
        for (auto& val : mesures) val /= passes;
        return mesures;
    }
}

/*
TEST_CASE("performances fluent_collections vs lazy_ranges vs brut code", "[.][performances]") {

    std::vector<int> values(100'000'000);
    for (int i = 0; i < 100'000; ++i) {
        values[i] = i;
    }

    auto predicate = [] (int val) { return val >= 30'000'000; };
    auto modulo = [] (int val) { return val % 17; };
    auto accumulate = [] (int v1, int v2) { return v1 + v2; };

    auto lazy_task = [=] {
        auto result = sc::lazy_range(values)
            .filter(predicate)
            .map   (modulo)
            .reduce(accumulate);
    };
    auto fluent_task = [=] {
        auto result = sc::fluent<std::vector, int>(values)
            .filter(predicate)
            .map   (modulo)
            .reduce(accumulate);
    };
    auto brut_task = [=] {
        int sum = 0;
        for (auto val : values) {
            if (!predicate(val)) continue;
            val = modulo(val);
            sum = accumulate(sum, val);
        }
    };

    auto times = mesure_tasks({ lazy_task, fluent_task, brut_task });

    std::cout << "\n       +------------------------+";
    std::cout << "\n       | fluent vs lazy vs brut |";
    std::cout << "\n       +------------------------+";
    std::cout << "\n";
    std::cout << "\n Lazy time :   " << times[0];
    std::cout << "\n Fluent time : " << times[1];
    std::cout << "\n Brut time :   " << times[2];
    std::cout << "\n";
}

TEST_CASE("performances mutex vs spin_lock", "[.][performances]") {
    constexpr int incCount(10'000);

    auto thread_task = [] (auto& lockable) {
        using lock_t = std::lock_guard<decltype(lockable)>;
        int sum = 0;

        auto thread = [=, &lockable, &sum] {
            int lastValue = -1;
            for (int i = 0; i < incCount; ++i) {
                lock_t lock(lockable);
                if (lastValue != sum) {
                    lastValue = ++sum;
                }
            }
        };

        std::vector<std::thread> threads;
        for (int t = 0; t < std::thread::hardware_concurrency(); ++t) {
            threads.emplace_back(thread);
        }
        for (auto& t : threads) t.join();
    };

    sc::spin_lock spinLock;
    std::mutex mutex;
    auto times = mesure_tasks({
        [&] { thread_task(mutex); },
        [&] { thread_task(spinLock); }
    });

    std::cout << "\n       +--------------------+";
    std::cout << "\n       | spin_lock vs mutex |";
    std::cout << "\n       +--------------------+";
    std::cout << "\n";
    std::cout << "\n mutex time :     " << times[0];
    std::cout << "\n spin_lock time : " << times[1];
    std::cout << "\n";
}

TEST_CASE("locked std::queue vs mpsc_queue", "[.][performances]") {
    constexpr int incCount(20'000);
    using data_t = std::array<int, 100>;

    auto locked_task = [&] () {
        std::queue<data_t> stdQueue;
        std::mutex mutex;
        auto thread = [&] {
            for (int i = 0; i < incCount; ++i) {
                std::lock_guard<std::mutex> lock{mutex};
                stdQueue.emplace();
            }
        };
        std::vector<std::thread> threads;
        for (int t = 0; t < std::thread::hardware_concurrency(); ++t) {
            threads.emplace_back(thread);
        }
        int count = incCount * std::thread::hardware_concurrency();
        while (count != 0) {
            bool empty;
            {
                std::lock_guard<std::mutex> lock{mutex};
                empty = stdQueue.empty();
            }
            if (!empty) {
                std::lock_guard<std::mutex> lock{mutex};
                stdQueue.front();
                stdQueue.pop();
                --count;
            }
        }
        for (auto& t : threads) t.join();
    };
    auto lockfree_task = [&] () { // TODO Reduce mpsc_queue capacity crashes or blocks
        sc::mpsc_queue<data_t> scQueue(incCount * std::thread::hardware_concurrency());

        int pSum = 0;
        auto thread = [&] {
            for (int i = 0; i < incCount; ++i) {
                scQueue.emplace();
            }
        };
        std::vector<std::thread> threads;
        for (int t = 0; t < std::thread::hardware_concurrency(); ++t) {
            threads.emplace_back(thread);
        }
        int count = incCount * std::thread::hardware_concurrency();
        while (count != 0) {
            scQueue.consume_all([&] (data_t&&) {--count;});
        }
        for (auto& t : threads) t.join();
    };

    auto times = mesure_tasks({locked_task, lockfree_task});

    std::cout << "\n       +----------------------------+";
    std::cout << "\n       | locked queue vs mpsc_queue |";
    std::cout << "\n       +----------------------------+";
    std::cout << "\n";
    std::cout << "\n std::queue + mutex time : " << times[0];
    std::cout << "\n sc::mpsc_queue time :     " << times[1];
    std::cout << "\n";
}

TEST_CASE("performances mutex vs spin_lock", "[.][performances]") {
    constexpr int incCount(10'000);

    auto thread_task = [] (auto& lockable) {
        using lock_t = std::lock_guard<decltype(lockable)>;
        int sum = 0;

        auto thread = [=, &lockable, &sum] {
            int lastValue = -1;
            for (int i = 0; i < incCount; ++i) {
                lock_t lock(lockable);
                if (lastValue != sum) {
                    lastValue = ++sum;
                }
            }
        };

        std::vector<std::thread> threads;
        for (int t = 0; t < std::thread::hardware_concurrency(); ++t) {
            threads.emplace_back(thread);
        }
        for (auto& t : threads) t.join();
    };

    sc::spin_lock spinLock;
    std::mutex mutex;
    auto times = mesure_tasks({
                                      [&] { thread_task(mutex); },
                                      [&] { thread_task(spinLock); }
                              });

    std::cout << "\n       +--------------------+";
    std::cout << "\n       | spin_lock vs mutex |";
    std::cout << "\n       +--------------------+";
    std::cout << "\n";
    std::cout << "\n mutex time :     " << times[0];
    std::cout << "\n spin_lock time : " << times[1];
    std::cout << "\n";
}
*/
TEST_CASE("pod_vector vs std::vector (bytes)", "[.][performances]") {

    auto vector_task = [] (auto vector) {
        auto vec = vector;
        for (int i = 0; i < 100'000; ++i) {
            auto v = vec;
            v.resize(10'000);
            auto v2 = v;
        }
    };

    auto times = mesure_tasks({
        [=] { vector_task(std::vector<char>{}); },
        [=] { vector_task(sc::pod_vector<char>{}); }
    });

    std::cout << "\n       +-----------------------------------+";
    std::cout << "\n       | pod_vector vs std::vector (bytes) |";
    std::cout << "\n       +-----------------------------------+";
    std::cout << "\n";
    std::cout << "\n std::vector resize and copies : " << times[0];
    std::cout << "\n pod_vector resize and copies :  " << times[1];
    std::cout << "\n";
}
