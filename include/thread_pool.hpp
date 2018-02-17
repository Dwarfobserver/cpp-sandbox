
#pragma once

#include <thread>
#include <vector>
#include <condition_variable>
#include <deque>
#include <atomic>
#include <future>
#include "movable_function.hpp"


namespace sc {

    class thread_pool {
    public:
        explicit thread_pool(int threadsCount);
        ~thread_pool();

        thread_pool(thread_pool&&) = delete;

        template <class F>
        auto execute(F&& f) {
            using return_t = decltype(f());

            std::promise<return_t> promise;
            auto future = promise.get_future();
            {
                std::lock_guard lock{tasksMutex_};
                tasks_.emplace_back([promise = std::move(promise), task = std::forward<F>(f)] () mutable {
                    promise.set_value(task());
                });
            }
            conditionVariable_.notify_one();
            return future;
        }
    private:
        void worker_loop();

        std::vector<std::thread> threads_;

        std::condition_variable conditionVariable_;

        std::deque<sc::movable_function<void()>> tasks_;
        std::mutex tasksMutex_;

        std::atomic_bool interrupting_;
    };

}
