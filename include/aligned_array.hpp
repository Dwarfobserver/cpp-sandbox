
#pragma once

#include "utils.hpp"
#include <bits/allocator.h>
#include <cstddef>


namespace sc {

    /// Allocates an array aligned in memory, without constructing values (use placement new).
    template<class T, template <class> class alloc_t = std::allocator>
    struct aligned_array {
        using alloc_bytes = alloc_t<std::byte>;

        size_t size;
        std::byte* unalignedData;
        T* data;

        T* allocate(size_t count, size_t alignment, bool endingPad);

        aligned_array() noexcept;
        ~aligned_array() noexcept;

        aligned_array(aligned_array && old) noexcept;
        aligned_array<T, alloc_t>& operator=(aligned_array && old) noexcept;

        aligned_array(aligned_array const& clone) = delete;
        aligned_array& operator=(aligned_array const& clone) = delete;
    private:
        size_t init() noexcept;
        size_t copy(aligned_array && old) noexcept;
    };

    // ______________
    // Implementation

    template<class T, template <class> class alloc_t>
    T* aligned_array<T, alloc_t>::allocate(size_t count, size_t alignment, bool endingPad) {
        if (unalignedData != nullptr) {
            alloc_bytes().deallocate(unalignedData, size);
        }

        if (alignof(T) > alignment) alignment = alignof(T);
        size = sizeof(T) * count + alignment * (1 + endingPad);

        unalignedData = alloc_bytes().allocate(size);

        auto shiftedPtr = ((reinterpret_cast<intptr_t>(unalignedData) + alignment - 1) / alignment) * alignment;
        data = reinterpret_cast<T*>(shiftedPtr);
        return data;
    }

    template<class T, template <class> class alloc_t>
    aligned_array<T, alloc_t>::aligned_array() noexcept :
            size(init())
    {}

    template<class T, template <class> class alloc_t>
    aligned_array<T, alloc_t>::~aligned_array() noexcept {
        alloc_bytes().deallocate(unalignedData, size);
    }

    template<class T, template <class> class alloc_t>
    aligned_array<T, alloc_t>::aligned_array(aligned_array &&old) noexcept :
            size(copy(std::move(old)))
    {
        old.init();
    }

    template<class T, template <class> class alloc_t>
    aligned_array<T, alloc_t>& aligned_array<T, alloc_t>::operator=(aligned_array &&old) noexcept {
        copy(std::move(old));
        old.init();
        return *this;
    }

    template<class T, template <class> class alloc_t>
    size_t aligned_array<T, alloc_t>::init() noexcept {
        size = 0;
        unalignedData = nullptr;
        data = nullptr;
        return size;
    }

    template<class T, template <class> class alloc_t>
    size_t aligned_array<T, alloc_t>::copy(aligned_array &&old) noexcept {
        size = old.size;
        unalignedData = old.unalignedData;
        data = old.data;
        return size;
    }

} // End of ::sc
