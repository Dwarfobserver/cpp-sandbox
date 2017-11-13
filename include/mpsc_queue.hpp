
#pragma once

#include <utils.hpp>

#include <bits/allocator.h>
#include <atomic>


namespace sc {

    template<class T, template<class> class Allocator = std::allocator>
    class mpsc_queue {
        using buffer_alloc_t = sc::aligned_allocator<T, Allocator, SC_CACHE_LINE_SIZE>;
    public:
        explicit mpsc_queue(int _capacity);
        ~mpsc_queue() noexcept;

        template <class...Args>
        void emplace(Args &&...args);

        inline void push(T&& moved) { emplace(std::move(moved)); }
        inline void push(T const &clone) { emplace(const_cast<T&>(clone)); }

        template <class F>
        void consume(F&& f);
        template <class F>
        int consume_all(F&& f);

        mpsc_queue(mpsc_queue const& clone) = delete;
        mpsc_queue& operator=(mpsc_queue const& clone) = delete;
        mpsc_queue(mpsc_queue && moved) = delete;
        mpsc_queue& operator=(mpsc_queue && moved) = delete;
    private:
        // Const values
        alignas(SC_CACHE_LINE_SIZE) const int capacity;
        T* buffer;
        // Value set by consumer
        alignas(SC_CACHE_LINE_SIZE) std::atomic<int> nextPos;
        // Values set by producers
        alignas(SC_CACHE_LINE_SIZE)  std::atomic<int> head;
        std::atomic<int> tail;
        const std::byte cacheLineBytes[SC_CACHE_LINE_SIZE - sizeof(std::atomic<int>) * 2];
    };

    // ______________
    // Implementation

    static int upper_power_of_two(int val) {
        int power = 1;
        while (power < val) power *= 2;
        return power;
    }

    template<class T, template<class> class Allocator>
    mpsc_queue<T, Allocator>::mpsc_queue(int _capacity) :
            capacity(upper_power_of_two(_capacity + 1)),
            buffer(nullptr),
            nextPos(0),
            head(0),
            tail(0),
            cacheLineBytes{}
    {
        static_assert(alignof(T) <= SC_CACHE_LINE_SIZE, "T alignment must not be superior to cache line size");
        if (_capacity <= 0) {
            throw std::runtime_error{"mpsc_queue capacity must be superior to zero."};
        }

        buffer = buffer_alloc_t().allocate(capacity);
    }

    template<class T, template<class> class Allocator>
    mpsc_queue<T, Allocator>::~mpsc_queue() noexcept {
        if (buffer != nullptr) {
            // Call stored t's destructors
            consume_all([](T &&) {});
            buffer_alloc_t().deallocate(buffer, capacity);
        }
    }

    template<class T, template<class> class Allocator> template<class...Args>
    void mpsc_queue<T, Allocator>::emplace(Args &&... args) {
        const int pos = nextPos.fetch_add(1, std::memory_order_acquire) & ~capacity;

        // Wait empty place
        const int nextPos = (pos + 1) & ~capacity;
        while (nextPos == tail.load());

        new (buffer + pos) T(std::forward<Args>(args)...);

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

    template<class T, template<class> class Allocator> template<class F>
    void mpsc_queue<T, Allocator>::consume(F &&f) {
        const auto data = buffer;
        int i = tail.load(std::memory_order_acquire);
        while (i == head.load(std::memory_order_acquire));
        f(std::move(data[i]));
        data[i].~T();
        tail.store((i + 1) & ~capacity);
    }

    template<class T, template<class> class Allocator> template<class F>
    int mpsc_queue<T, Allocator>::consume_all(F &&f) {
        const auto data = buffer;
        const auto iMin = tail.load(std::memory_order_acquire);
        const auto iMax = head.load(std::memory_order_acquire);
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
