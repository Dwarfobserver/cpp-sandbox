
#pragma once

#include <cstddef>
#include <stdexcept>
#include <vector>


namespace sc {

    template <class T, size_t ALIGN = sizeof(T)> // TAG ?
    class block_allocator {
    public:
        using value_type = T;
        using pointer = T*;
        using const_pointer = T const*;
        using reference = T&;
        using const_reference = T const&;
        using size_type = size_t;
        using difference_type = ptrdiff_t;

        // Must not be rebind

        template <template <class> class Allocator = std::allocator>
        void allocate_blocks(size_t nb);

        template <template <class> class Allocator = std::allocator>
        void deallocate_blocks();

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
        static size_t chunksCount;
    };

    // ______________
    // Implementation

    template <class T, size_t ALIGN> template <template <class> class Allocator>
    void block_allocator<T, ALIGN>::allocate_blocks(size_t nb) {
        static_assert(sizeof(T) <= ALIGN, "T must have a lower size than the alignment required.");
        if (chunks != nullptr)
            throw std::runtime_error{"Tried to allocate already allocated blocks."};

        chunksCount = nb;
        chunks = Allocator<chunk_t>().allocate(nb);
        for (int i = 0; i < nb - 1; ++i) {
            chunks[i].next = (chunks + i + 1);
        }
        chunks[nb - 1].next = nullptr;
        next = chunks;
    }

    template <class T, size_t ALIGN> template <template <class> class Allocator>
    void block_allocator<T, ALIGN>::deallocate_blocks() {
        static_assert(sizeof(T) <= ALIGN, "T must have a lower size than the alignment required.");
        if (chunks == nullptr)
            throw std::runtime_error{"Tried to deallocate blocks not allocated."};

        Allocator<chunk_t>().deallocate(chunks, chunksCount);
        chunks = nullptr;
        chunksCount = 0;
    }

    template <class T, size_t ALIGN>
    T* block_allocator<T, ALIGN>::allocate(size_t nb) {
        static_assert(sizeof(T) <= ALIGN, "T must have a lower size than the alignment required.");
#ifndef NDEBUG
        if (nb != 1) throw std::runtime_error
                    {"block_allocator can only allocate one block at a time."};

        if (next == nullptr) throw std::runtime_error
                    {"block_allocator has exceeded his capacity of " + std::to_string(chunksCount) + "."};
#endif
        chunk_t* result = next;
        next = next->next;
        return reinterpret_cast<T*>(result);
    }

    template <class T, size_t ALIGN>
    void block_allocator<T, ALIGN>::deallocate(T *pChunk, size_t nb) {
        static_assert(sizeof(T) <= ALIGN, "T must have a lower size than the alignment required.");
#ifndef NDEBUG
        if (nb != 1) throw std::runtime_error
                    {"block_allocator can only allocate one block at a time."};
#endif
        reinterpret_cast<chunk_t*>(pChunk)->next = next;
        next = reinterpret_cast<chunk_t*>(pChunk);
    }

    template <class T, size_t ALIGN>
    typename block_allocator<T, ALIGN>::chunk_t*
             block_allocator<T, ALIGN>::chunks = nullptr;

    template <class T, size_t ALIGN>
    typename block_allocator<T, ALIGN>::chunk_t*
             block_allocator<T, ALIGN>::next = nullptr;

    template <class T, size_t ALIGN>
    size_t block_allocator<T, ALIGN>::chunksCount = 0;

}
