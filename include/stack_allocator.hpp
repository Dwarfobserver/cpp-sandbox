
#pragma once

#include <cstddef>
#include <malloc.h>
#include <type_traits>


namespace sc {

    template<class T>
    class stack_allocator {
    public:
        using value_type = T;
        using propagate_on_container_move_assignment = std::false_type;
        using propagate_on_container_copy_assignment = std::true_type;
        using is_always_equal = std::true_type;

        template <class U>
        struct rebind { using other = stack_allocator<U>; };

        T *allocate(size_t nb) {
            return reinterpret_cast<T*>(alloca(sizeof(T) * nb));
        }
        void deallocate(T *pChunk, size_t nb) {}
    };

    template <class T, class U>
    bool operator==(stack_allocator<T> const& lhs, stack_allocator<U> const& rhs) { return true; };
    template <class T, class U>
    bool operator!=(stack_allocator<T> const& lhs, stack_allocator<U> const& rhs) { return false; };

}
