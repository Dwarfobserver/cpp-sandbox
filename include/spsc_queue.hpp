
#pragma once

#include <aligned_array.hpp>

#include <atomic>


namespace sc {

    template<class T, template <class> class alloc_t = std::allocator>
    class spsc_queue {
    public:
        explicit spsc_queue(int _capacity);
        ~spsc_queue() noexcept;

        int size() const noexcept;

        template<class...Args>
        void emplace(Args &&...args);

        void push(T&& moved) { emplace(std::move(moved)); }
        void push(T const &clone) { emplace(const_cast<T&>(clone)); }

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

        // Aligned to not need to read other threads cache lines
        alignas(SC_CACHE_LINE_SIZE) const int capacity;
        aligned_array<T, alloc_t> ringBuffer;

        alignas(SC_CACHE_LINE_SIZE) int tail;
        alignas(SC_CACHE_LINE_SIZE) std::atomic_int head;
    };

    // ______________
    // Implementation

    template<class T, template <class> class alloc_t>
    spsc_queue<T, alloc_t>::spsc_queue(int _capacity) :
            capacity(_capacity + 1), tail(0), head(0) {
        static_assert(alignof(T) <= SC_CACHE_LINE_SIZE, "T alignment must not be superior to cache line size");
        if (_capacity < 1) {
            throw std::invalid_argument{"spsc_queue capacity must be >= 1."};
        }

        ringBuffer.allocate(static_cast<size_t>(capacity), SC_CACHE_LINE_SIZE, true);
    }

    template<class T, template <class> class alloc_t>
    spsc_queue<T, alloc_t>::~spsc_queue() noexcept {
        // Call stored t's destructors
        consume_all([](T &&) {});
    }

    template<class T, template <class> class alloc_t>
    int spsc_queue<T, alloc_t>::size() const noexcept {
        const auto _head = head.load(std::memory_order_consume);
        return _head - tail + capacity * (_head < tail);
    }

    template<class T, template <class> class alloc_t> template <class...Args>
    void spsc_queue<T, alloc_t>::emplace(Args &&... args) {
        static_assert(std::is_constructible_v<T, Args...>);

        const auto i = head.load(std::memory_order_consume);
        const auto i2 = i == capacity - 1 ? 0 : i + 1;

        #ifndef NDEBUG
        if (i2 == tail) throw std::runtime_error{"The producer has overflowed the spsc_queue."};
        std::atomic_thread_fence(std::memory_order_seq_cst);
        #endif

        new (ringBuffer.data + i) T(std::forward<Args>(args)...);

        head.store(i2, std::memory_order_release);
    }

    template<class T, template <class> class alloc_t> template <class F>
    int spsc_queue<T, alloc_t>::consume_all(F &&f) {
        const auto data = ringBuffer.data;
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

    template<class T, template <class> class alloc_t> template <class F>
    void spsc_queue<T, alloc_t>::consume_range(T *begin, T *end, F &&f) {
        while (begin != end) {
            f(std::move(*begin));
            begin->~T();
            ++begin;
        }
    }

} // End of ::sc
