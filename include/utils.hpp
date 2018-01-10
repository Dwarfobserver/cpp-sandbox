
#pragma once

#include <cstddef>
#include <optional>
#include <atomic>


/// Defines SC_CACHE_LINE_SIZE to use structures based on custom cache line size.
// TODO Get std::hardware_destructive_interference_size from a modern compiler
#ifndef SC_CACHE_LINE_SIZE
#define SC_CACHE_LINE_SIZE 64
#endif


namespace sc {
    template <class F>
    class defer {
    public:
        explicit defer(F&& f) : callback(std::move(f)) {}
        ~defer() { callback(); }
    private:
        F callback;
    };

    class spin_lock {
        std::atomic<bool> spin;
    public:
        spin_lock() : spin(false) {}
        bool try_lock() {
            return !spin.exchange(true, std::memory_order_acquire);
        }
        void lock() {
            bool exceptedValue;
            do {
                exceptedValue = false;
            } while (!spin.compare_exchange_weak(exceptedValue, true, std::memory_order_acquire));
        }
        void unlock() {
            spin.store(false, std::memory_order_release);
        };
    };

    template<class T, template <class> class Allocator, int ALIGN>
    class aligned_allocator {
        struct alignas(ALIGN) chunk_t {
        private:
            std::byte bytes[ALIGN];
        };
    public:
        using value_type = T;

        template<typename U>
        struct rebind {
            using other = aligned_allocator<U, Allocator, ALIGN>;
        };

        T* allocate(size_t nb) {
            static_assert(alignof(T) <= ALIGN, "T must have a lower alignment constraint than the alignment required.");
            return reinterpret_cast<T*>(Allocator<chunk_t>().allocate((sizeof(T) * nb + ALIGN - 1) / ALIGN));
        }

        void deallocate(T* pChunk, size_t nb) {
            static_assert(alignof(T) <= ALIGN, "T must have a lower alignment constraint than the alignment required.");
            Allocator<chunk_t>().deallocate(reinterpret_cast<chunk_t*>(pChunk), (sizeof(T) * nb + ALIGN - 1) / ALIGN);
        }
    };

}
