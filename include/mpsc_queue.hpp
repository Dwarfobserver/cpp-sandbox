
#pragma once

#include <bits/allocator.h>
#include <atomic>


#ifndef SC_CACHE_LINE_SIZE
#define SC_CACHE_LINE_SIZE 64
#endif

namespace sc {

    // TODO Rare slowdowns encoutered in tests

    template<class T, class Allocator = std::allocator<T>>
    class mpsc_queue {
    public:
        explicit mpsc_queue(int capacity, Allocator const& allocator = Allocator());
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
        alignas(SC_CACHE_LINE_SIZE) const int capacity_;
        T* buffer_;
        Allocator allocator_;
        // Value set by consumer
        alignas(SC_CACHE_LINE_SIZE) std::atomic<int> nextPos_;
        // Values set by producers
        alignas(SC_CACHE_LINE_SIZE)  std::atomic<int> head_;
        std::atomic<int> tail_;
        const std::byte cacheLineBytes_[SC_CACHE_LINE_SIZE - sizeof(std::atomic<int>) * 2];
    };

    // ______________
    // Implementation

    static int upper_power_of_two(int val) {
        int power = 1;
        while (power < val) power *= 2;
        return power;
    }

    template<class T, class Allocator>
    mpsc_queue<T, Allocator>::mpsc_queue(int capacity, Allocator const& allocator) :
            capacity_(upper_power_of_two(capacity + 1)),
            buffer_(nullptr),
            allocator_(allocator),
            nextPos_(0),
            head_(0),
            tail_(0),
            cacheLineBytes_{}
    {
        static_assert(alignof(T) <= SC_CACHE_LINE_SIZE, "T alignment must not be superior to cache line size");
        if (capacity <= 0) {
            throw std::runtime_error{"mpsc_queue capacity must be superior to zero."};
        }

        buffer_ = std::allocator_traits<Allocator>::allocate(allocator_, capacity_);
    }

    template<class T, class Allocator>
    mpsc_queue<T, Allocator>::~mpsc_queue() noexcept {
        if (buffer_ != nullptr) {
            // Call stored t's destructors
            consume_all([](T &&) {});
            std::allocator_traits<Allocator>::deallocate(allocator_, buffer_, capacity_);
        }
    }

    template<class T, class Allocator> template<class...Args>
    void mpsc_queue<T, Allocator>::emplace(Args &&... args) {
        const int pos = nextPos_.fetch_add(1, std::memory_order_acquire) & ~capacity_;

        // Wait empty place
        const int nextPos = (pos + 1) & ~capacity_;
        while (nextPos == tail_.load());

        new (buffer_ + pos) T(std::forward<Args>(args)...);

        int expectedPos;
        do {
            expectedPos = pos;
        } while (!head_.compare_exchange_weak(expectedPos, nextPos, std::memory_order_release));
    }

    namespace {
        template<class T, class F>
        static void consume_range(T *begin, T *end, F &&f) {
            while (begin != end) {
                f(std::move(*begin));
                begin->~T();
                ++begin;
            }
        }
    }

    template<class T, class Allocator> template<class F>
    void mpsc_queue<T, Allocator>::consume(F &&f) {
        const auto data = buffer_;
        int i = tail_.load(std::memory_order_acquire);
        while (i == head_.load(std::memory_order_acquire));
        f(std::move(data[i]));
        data[i].~T();
        tail_.store((i + 1) & ~capacity_);
    }

    template<class T, class Allocator> template<class F>
    int mpsc_queue<T, Allocator>::consume_all(F &&f) {
        const auto data = buffer_;
        const auto iMin = tail_.load(std::memory_order_acquire);
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
        tail_.store(iMax, std::memory_order_release);
        return count;
    }

}
