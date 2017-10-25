
#pragma once

#include <cstddef>
#include <stdexcept>
#include <vector>


namespace sc {

    template <class T, template <class> class Allocator, size_t COUNT, size_t ALIGN = sizeof(T)>
    class block_allocator {
    public:
        using value_type = T;
        using pointer = T*;
        using const_pointer = T const*;
        using reference = T&;
        using const_reference = T const&;
        using size_type = size_t;
        using difference_type = ptrdiff_t;

        template<typename U>
        struct rebind {
        private:
            constexpr size_t otherAlign() {
                if constexpr (sizeof(T) == ALIGN) {
                    return sizeof(U);
                }
                else {
                    static_assert(sizeof(U) <= ALIGN);
                    return ALIGN;
                }
            }
        public:
            using other = block_allocator<U, Allocator, COUNT, otherAlign()>;
        };

        T* allocate(size_t nb);
        void deallocate(T* pChunk, size_t nb);

    private:
        struct alignas(ALIGN) chunk_t {
            union {
                chunk_t* next;
                std::byte bytes[ALIGN];
            };
        };
        static chunk_t* chunks;
        static chunk_t* next;
    };

    // ______________
    // Implementation

    template <class T, template <class> class Allocator, size_t COUNT, size_t ALIGN>
    T* block_allocator<T, Allocator, COUNT, ALIGN>::allocate(size_t nb) {
        static_assert(sizeof(T) <= ALIGN, "T must have a lower size than the alignment required.");
#ifndef NDEBUG
        if (nb != 1) throw std::runtime_error
                    {"block_allocator can only allocate one block at a time."};
#endif
        // Allocate chunks
        if (chunks == nullptr) {
            chunks = Allocator<chunk_t>().allocate(COUNT);
            for (int i = 0; i < COUNT - 1; ++i) {
                chunks[i].next = (chunks + i + 1);
            }
            chunks[COUNT - 1].next = nullptr;
            next = chunks;
        }
#ifndef NDEBUG
        if (next == nullptr) throw std::runtime_error
                    {"block_allocator has exceeded his capacity of " + std::to_string(COUNT) + "."};
#endif
        chunk_t* result = next;
        next = next->next;
        return reinterpret_cast<T*>(result);
    }

    template <class T, template <class> class Allocator, size_t COUNT, size_t ALIGN>
    void block_allocator<T, Allocator, COUNT, ALIGN>::deallocate(T *pChunk, size_t nb) {
        static_assert(sizeof(T) <= ALIGN, "T must have a lower size than the alignment required.");
#ifndef NDEBUG
        if (nb != 1) throw std::runtime_error
                    {"block_allocator can only allocate one block at a time."};
#endif
        reinterpret_cast<chunk_t*>(pChunk)->next = next;
        next = reinterpret_cast<chunk_t*>(pChunk);
    }

    template <class T, template <class> class Allocator, size_t COUNT, size_t ALIGN>
    typename block_allocator<T, Allocator, COUNT, ALIGN>::chunk_t*
             block_allocator<T, Allocator, COUNT, ALIGN>::chunks = nullptr;

    template <class T, template <class> class Allocator, size_t COUNT, size_t ALIGN>
    typename block_allocator<T, Allocator, COUNT, ALIGN>::chunk_t*
             block_allocator<T, Allocator, COUNT, ALIGN>::next = nullptr;

}
