
#ifndef SIDNEY_MPSC_QUEUE_HPP
#define SIDNEY_MPSC_QUEUE_HPP

#include "utils.hpp"
#include <atomic>


namespace sc {

    template<class T>
    class spsc_queue {
    public:
        spsc_queue(spsc_queue const&) = delete;
        spsc_queue& operator=(spsc_queue const&) = delete;
        spsc_queue(spsc_queue &&) = delete;
        spsc_queue& operator=(spsc_queue &&) = delete;

        explicit spsc_queue(int _capacity);
        ~spsc_queue();

        template<class...Args>
        void emplace(Args &&...args);

        void push(T const &clone);

        /// Apply a function which consumes each available data and returns the number of executions.
        template<class F>
        int consumeAll(F &&f);
    private:
        template<class F>
        static void consumeRange(T *begin, T *end, F &&f);

        // Aligned to not need to read other threads cache lines
        alignas(SC_CACHE_LINE_SIZE) const int capacity;
        utils::aligned_array<T> ringBuffer;

        alignas(SC_CACHE_LINE_SIZE) int tail;
        alignas(SC_CACHE_LINE_SIZE) std::atomic_int head;
    };

    // ______________
    // Implementation

    template<class T>
    spsc_queue<T>::spsc_queue(int _capacity) :
            capacity(_capacity + 1), tail(0), head(0) {
        static_assert(alignof(T) <= SC_CACHE_LINE_SIZE, "T alignment must not be superior to cache line size");
        static_assert(sizeof(ringBuffer) + sizeof(capacity) <= SC_CACHE_LINE_SIZE);
        if (capacity < 2) {
            throw std::invalid_argument{"spsc_queue capacity must be superior to 2."};
        }

        ringBuffer.allocate(capacity, SC_CACHE_LINE_SIZE, true);
    }

    template<class T> template <class F>
    void spsc_queue<T>::consumeRange(T *begin, T *end, F &&f) {
        while (begin != end) {
            f(std::move(*begin));
            begin->~T();
            ++begin;
        }
    }

    template<class T>
    spsc_queue<T>::~spsc_queue() {
        // Call stored t's destructors
        consumeAll([](T &&) {});
    }

    template<class T> template <class...Args>
    void spsc_queue<T>::emplace(Args &&... args) {
        auto i = head.load(std::memory_order_consume);

        new (ringBuffer.data + i) T(std::forward<Args>(args)...);
        i = i == capacity - 1 ? 0 : i + 1;

        #ifndef NDEBUG
        std::atomic_thread_fence(std::memory_order_seq_cst);
        if (i == tail) throw std::length_error{"The producer has overflowed the spsc_queue."};
        #endif

        head.store(i, std::memory_order_release);
    }

    template<class T>
    void spsc_queue<T>::push(T const &clone) {
        emplace(clone);
    }

    template<class T> template <class F>
    int spsc_queue<T>::consumeAll(F &&f) {
        const auto data = ringBuffer.data;
        const auto iMin = tail;
        // consumeRange(iMin -> capacity) isn't dependant
        const auto iMax = head.load(std::memory_order_acquire);
        // Careful of not use unsigned
        auto count = iMax - iMin;

        if (iMin <= iMax) {
            consumeRange(data + iMin, data + iMax, f);
        } else {
            consumeRange(data + iMin, data + capacity, f);
            consumeRange(data, data + iMax, f);
            count += capacity;
        }
        tail = iMax;
        return count;
    }

} // End of ::sc

#endif
