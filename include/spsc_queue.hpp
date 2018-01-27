
#pragma once

#include <atomic>


#ifndef SC_CACHE_LINE_SIZE
#define SC_CACHE_LINE_SIZE 64
#endif

namespace sc {

    template<class T, class Allocator = std::allocator<T>>
    class spsc_queue {
    public:
        explicit spsc_queue(int capacity, Allocator const& allocator = Allocator());
        ~spsc_queue() noexcept;

        template<class...Args>
        void emplace(Args &&...args);

        inline void push(T&& moved) { emplace(std::move(moved)); }
        inline void push(T const &clone) { emplace(clone); }

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
        alignas(SC_CACHE_LINE_SIZE) const int capacity_;
        T* buffer_;
        Allocator allocator_;
        // Value set by consumer
        alignas(SC_CACHE_LINE_SIZE) int tail_;
        // Value set by producer
        alignas(SC_CACHE_LINE_SIZE) std::atomic<int> head_;
        const std::byte cacheLineBytes_[SC_CACHE_LINE_SIZE - sizeof(std::atomic<int>)];
    };

    // ______________
    // Implementation

    namespace {
        int upper_power_of_two(int val) {
            int power = 1;
            while (power < val) power *= 2;
            return power;
        }
    }

    template<class T, class Allocator>
    spsc_queue<T, Allocator>::spsc_queue(int capacity, Allocator const& allocator) :
            capacity_(upper_power_of_two(capacity + 1)),
            buffer_(nullptr),
            allocator_(allocator),
            tail_(0),
            head_(0),
            cacheLineBytes_{}
    {
        static_assert(alignof(T) <= SC_CACHE_LINE_SIZE, "T alignment must not be superior to cache line size");
        if (capacity <= 0) {
            throw std::invalid_argument{"spsc_queue capacity must be superior to zero."};
        }

        buffer_ = std::allocator_traits<Allocator>::allocate(allocator_, capacity_);
    }

    template<class T, class Allocator>
    spsc_queue<T, Allocator>::~spsc_queue() noexcept {
        if (buffer_ != nullptr) {
            // Call stored t's destructors
            consume_all([](T &&) {});
            std::allocator_traits<Allocator>::deallocate(allocator_, buffer_, capacity_);
        }
    }

    template<class T, class Allocator> template <class...Args>
    void spsc_queue<T, Allocator>::emplace(Args &&... args) {
        static_assert(std::is_constructible_v<T, Args...>);

        const auto i = head_.load(std::memory_order_consume);
        const auto i2 = (i + 1) & (capacity_ - 1);

#ifndef NDEBUG
        if (i2 == tail_) throw std::runtime_error{"The producer has overflowed the spsc_queue."};
        std::atomic_thread_fence(std::memory_order_seq_cst);
#endif

        new (buffer_ + i) T(std::forward<Args>(args)...);

        head_.store(i2, std::memory_order_release);
    }

    template<class T, class Allocator> template <class F>
    int spsc_queue<T, Allocator>::consume_all(F &&f) {
        const auto data = buffer_;
        const auto iMin = tail_;
        // consume_range(iMin -> capacity) isn't dependant
        const auto iMax = head_.load(std::memory_order_acquire);
        // Careful of not use unsigned
        auto count = iMax - iMin;

        if (iMin <= iMax) {
            consume_range(data + iMin, data + iMax, f);
        } else {
            consume_range(data + iMin, data + capacity_, f);
            consume_range(data, data + iMax, f);
            count += capacity_;
        }
        tail_ = iMax;
        return count;
    }

    template<class T, class Allocator> template <class F>
    void spsc_queue<T, Allocator>::consume_range(T *begin, T *end, F &&f) {
        while (begin != end) {
            f(std::move(*begin));
            begin->~T();
            ++begin;
        }
    }

} // End of ::sc
