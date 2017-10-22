
#pragma once

#include <bits/allocator.h>
#include <atomic>
#include "aligned_array.hpp"

namespace sc {

    template<class T, template<class> class alloc_t = std::allocator>
    class mpsc_queue {
    public:
        explicit mpsc_queue(int _capacity);
        ~mpsc_queue() noexcept;

        template <class...Args>
        void emplace(Args &&...args);

        void push(T&& moved) { emplace(std::move(moved)); }
        void push(T const &clone) { emplace(const_cast<T&>(clone)); }

        template <class F>
        int consume_all(F&& f);

        mpsc_queue(mpsc_queue const& clone) = delete;
        mpsc_queue& operator=(mpsc_queue const& clone) = delete;
        mpsc_queue(mpsc_queue && moved) = delete;
        mpsc_queue& operator=(mpsc_queue && moved) = delete;
    private:
        alignas(SC_CACHE_LINE_SIZE) sc::aligned_array<T, alloc_t> buffer;
        int capacity;
        alignas(SC_CACHE_LINE_SIZE) std::atomic<int> nextPos;
        std::atomic<int> head;
        std::atomic<int> tail;
    };

    // ______________
    // Implementation

    static int upper_power_of_two(int val) {
        int power = 1;
        while (power < val) power *= 2;
        return power;
    }

    template<class T, template<class> class alloc_t>
    mpsc_queue<T, alloc_t>::mpsc_queue(int _capacity) :
            capacity(upper_power_of_two(_capacity + 1)),
            nextPos(0),
            head(0),
            tail(0)
    {
        if (_capacity <= 0) {
            throw std::runtime_error{"mpsc_queue capacity must be superior to zero."};
        }

        buffer.allocate(static_cast<size_t>(capacity), SC_CACHE_LINE_SIZE, true);
    }

    template<class T, template<class> class alloc_t>
    mpsc_queue<T, alloc_t>::~mpsc_queue() noexcept {
        consume_all([] (T&&) {});
    }

    template<class T, template<class> class alloc_t> template<class...Args>
    void mpsc_queue<T, alloc_t>::emplace(Args &&... args) {
        const int pos = nextPos.fetch_add(1, std::memory_order_consume) & (capacity - 1);

        // Wait empty place
        const int nextPos = (pos + 1) & (capacity - 1);
        while (nextPos == tail.load());

        new (buffer.data + pos) T(std::forward<Args>(args)...);

        int expectedPos;
        do {
            expectedPos = pos;
        } while (!head.compare_exchange_weak(expectedPos, nextPos, std::memory_order_release));
    }

    template<class T, class F>
    static void consume_range(T *begin, T *end, F &&f) {
        while (begin != end) {
            f(std::move(*begin));
            begin->~T();
            ++begin;
        }
    }
    template<class T, template<class> class alloc_t> template<class F>
    int mpsc_queue<T, alloc_t>::consume_all(F &&f) {
        const auto data = buffer.data;
        const auto iMin = tail.load(std::memory_order_consume);
        const auto iMax = head.load(std::memory_order_consume);
        // Careful of not use unsigned
        auto count = iMax - iMin;

        if (iMin <= iMax) {
            consume_range(data + iMin, data + iMax, f);
        } else {
            consume_range(data + iMin, data + capacity, f);
            consume_range(data, data + iMax, f);
            count += capacity;
        }
        tail.store(iMax, std::memory_order_release);
        return count;
    }

} // End of ::sc
