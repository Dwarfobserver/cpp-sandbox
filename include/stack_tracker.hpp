
#pragma once

#include <cstdint>


namespace sc {

    namespace detail {
        extern intptr_t stack_address;
        extern int stack_capacity;
    }

#if defined(__clang__) || defined(__GNUC__) || defined(__GNUG__)
#define SC_ALWAYS_INLINE __attribute__((always_inline)) inline
#elif defined(_MSC_VER)
    #define SC_ALWAYS_INLINE __forceinline
#else
#error Compiler not supported
#endif

    SC_ALWAYS_INLINE void initialize_stack_tracker(int capacity) {
        {
            int stack_variable = 42;
            detail::stack_address = reinterpret_cast<intptr_t>(&stack_variable);
            detail::stack_capacity = capacity + stack_variable - 42; // Mastermind optimisation obfuscation !!
        }
    }

#undef SC_ALWAYS_INLINE

    inline bool is_stack_allocated(void* memoryAddress) {
        return reinterpret_cast<intptr_t>(memoryAddress) - detail::stack_address < detail::stack_capacity;
    }

}
