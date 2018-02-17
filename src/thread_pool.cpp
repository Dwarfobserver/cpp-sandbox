
#include <thread_pool.hpp>
#include <iostream>

namespace sc {

    thread_pool::thread_pool(int threadsCount) :
        interrupting_(false)
    {
        for (int i = 0; i < threadsCount; ++i) {
            threads_.emplace_back([this] {
                worker_loop();
            });
        }
    }

    void thread_pool::worker_loop() {
        sc::movable_function<void()> task;

        while (!interrupting_.load()) {
            {
                std::unique_lock lock{tasksMutex_};
                conditionVariable_.wait(lock, [this] {
                    return interrupting_.load() || !tasks_.empty();
                });
            }
            if (interrupting_.load()) break;
            {
                std::lock_guard lock2{tasksMutex_};
                task = std::move(tasks_.front());
                tasks_.pop_front();
            }
            task();
        }
    }

    thread_pool::~thread_pool() {
        interrupting_.store(true);
        conditionVariable_.notify_all();
        for (auto& thread : threads_) {
            thread.join();
        }
    }

}
