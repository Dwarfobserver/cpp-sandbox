
#pragma once

#include <utils.hpp>

#include <atomic>


namespace sc {

    template<class T, template <class> class Allocator = std::allocator>
    class spsc_queue {
        using buffer_alloc_t = sc::aligned_allocator<T, Allocator, SC_CACHE_LINE_SIZE>;
    public:
        explicit spsc_queue(int _capacity);
        ~spsc_queue() noexcept;

        template<class...Args>
        void emplace(Args &&...args);

        inline void push(T&& moved) { emplace(std::move(moved)); }
        inline void push(T const &clone) { emplace(const_cast<T&>(clone)); }

        // Apply a function which consumes each available data and returns the number of executions
        template<class F>
        int consume_all(F &&f);

        spsc_queue(spsc_queue const&) = delete;
        spsc_queue& operator=(spsc_queue const&) = delete;
        spsc_queue(spsc_queue &&) = delete;
        spsc_queue& operator=(spsc_queue &&) = delete;
    private:
        template<class F>
        static void consume_range(T *begin, T *end, F &&f);

        // Const values
        alignas(SC_CACHE_LINE_SIZE) const int capacity;
        T* buffer;
        // Value set by consumer
        alignas(SC_CACHE_LINE_SIZE) int tail;
        // Value set by producer
        alignas(SC_CACHE_LINE_SIZE) std::atomic<int> head;
        const std::byte cacheLineBytes[SC_CACHE_LINE_SIZE - sizeof(std::atomic<int>)];
    };

    // ______________
    // Implementation

    static int upper_power_of_two(int val) {
        int power = 1;
        while (power < val) power *= 2;
        return power;
    }

    template<class T, template <class> class Allocator>
    spsc_queue<T, Allocator>::spsc_queue(int _capacity) :
            capacity(upper_power_of_two(_capacity + 1)),
            buffer(nullptr),
            tail(0),
            head(0),
            cacheLineBytes{}
    {
        static_assert(alignof(T) <= SC_CACHE_LINE_SIZE, "T alignment must not be superior to cache line size");
        if (_capacity <= 0) {
            throw std::invalid_argument{"spsc_queue capacity must be superior to zero."};
        }

        buffer = buffer_alloc_t().allocate(capacity);
    }

    template<class T, template <class> class Allocator>
    spsc_queue<T, Allocator>::~spsc_queue() noexcept {
        if (buffer != nullptr) {
            // Call stored t's destructors
            consume_all([](T &&) {});
            buffer_alloc_t().deallocate(buffer, capacity);
        }
    }

    template<class T, template <class> class Allocator> template <class...Args>
    void spsc_queue<T, Allocator>::emplace(Args &&... args) {
        static_assert(std::is_constructible_v<T, Args...>);

        const auto i = head.load(std::memory_order_consume);
        const auto i2 = (i + 1) & (capacity - 1);

#ifndef NDEBUG
        if (i2 == tail) throw std::runtime_error{"The producer has overflowed the spsc_queue."};
        std::atomic_thread_fence(std::memory_order_seq_cst);
#endif

        new (buffer + i) T(std::forward<Args>(args)...);

        head.store(i2, std::memory_order_release);
    }

    template<class T, template <class> class Allocator> template <class F>
    int spsc_queue<T, Allocator>::consume_all(F &&f) {
        const auto data = buffer;
        const auto iMin = tail;
        // consume_range(iMin -> capacity) isn't dependant
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
        tail = iMax;
        return count;
    }

    template<class T, template <class> class Allocator> template <class F>
    void spsc_queue<T, Allocator>::consume_range(T *begin, T *end, F &&f) {
        while (begin != end) {
            f(std::move(*begin));
            begin->~T();
            ++begin;
        }
    }

} // End of ::sc
