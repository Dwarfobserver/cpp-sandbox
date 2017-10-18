
#pragma once

#include <bits/unique_ptr.h>


// TODO Get std::hardware_destructive_interference_size

#ifndef SC_CACHE_LINE_SIZE
#define SC_CACHE_LINE_SIZE 64
#endif

namespace sc::utils {
    /// Allocates an array aligned in memory, without constructing values (use placement new).
    // TODO Get std::aligned_alloc
    template<class T>
    struct aligned_array {
        std::unique_ptr<char[]> unalignedData;
        T* data;

        T* allocate(int count, int alignment, bool endingPad);

        aligned_array() : data(nullptr) {}
        aligned_array(aligned_array && old) noexcept :
                unalignedData(std::move(old.unalignedData)),
                data(old.data)
        {
            old.data = nullptr;
        }
        aligned_array& operator=(aligned_array && old) noexcept {
            unalignedData = std::move(old.unalignedData);
            data = old.data;
            old.data = nullptr;
            return *this;
        }
        aligned_array(aligned_array const& clone) = delete;
        aligned_array& operator=(aligned_array const& clone) = delete;
    };

    // ______________
    // Implementation

    template<class T>
    T* aligned_array<T>::allocate(int count, int alignment, bool endingPad) {
        auto bytes = sizeof(T) * count;
        auto shift = 0;
        if (alignof(T) < alignment) {
            shift = alignment - alignof(T);

            if (endingPad) {
                bytes += ((bytes + alignment - 1) / alignment) * alignment;
            }
        }
        unalignedData = std::make_unique<char[]>(bytes + shift);
        data = reinterpret_cast<T*>(unalignedData.get() + shift);
        return data;
    }

}
