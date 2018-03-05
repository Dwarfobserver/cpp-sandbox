
#pragma once

#if !defined(NDEBUG)
#include <string>
#include <stdexcept>
#endif

///FORCE_INLINE and  NO_INLINE : Indicates that a function must not be inlined.

#if defined(__clang__) || defined(__GNUG__)
    #define FORCE_INLINE inline __attribute__((always_inline))
    #define NO_INLINE __attribute__((noinline))
#elif defined(_MSC_VER)
    #define FORCE_INLINE __forceinline
    #define NO_INLINE __declspec(noinline)
#else
    #define FORCE_INLINE inline
    #define NO_INLINE
#endif

/// LIKELY(x) and UNLIKELY(x) : Mark an expression as evaluating to respectively true or false most of the time.

#if defined(__clang__) || defined(__GNUG__)
    #define LIKELY(x)   (__builtin_expect(static_cast<bool>(x), true))
    #define UNLIKELY(x) (__builtin_expect(static_cast<bool>(x), false))
#else
    #define LIKELY(x)   (static_cast<bool>(x))
    #define UNLIKELY(x) (static_cast<bool>(x))
#endif

/// RESTRICT : Indicates that pointers of the same type do not alias values.

#if defined(NDEBUG) && (defined(__clang__) || defined(__GNUG__) || defined(_MSC_VER))
    #define RESTRICT __restrict
#else
    #define RESTRICT
#endif

/// ASSERT(x, msg) : Ensures that x is evaluated to true. In debug, throw an exception.

#if defined(NDEBUG)
    #if defined(_MSC_VER)
        #define ASSERT(x, msg) __assume(x)
    #else
        #define ASSERT(x, msg) static_cast<void>(0)
    #endif
#else
namespace sc::detail {
    NO_INLINE void assert_failed(char const* expression, char const* file, int line, std::string const& message);
}
        #define ASSERT(x, msg) static_cast<void>( \
                                   LIKELY(x) || \
                                  (sc::detail::assert_failed(#x, __FILE__, static_cast<int>(__LINE__), msg), true))
#endif

/// UNREACHABLE() : Mark a path as unreachable. In debug, throw an exception.

#if defined(NDEBUG)
    #if defined(__clang__) || defined(__GNUG__)
        #define UNREACHABLE() __builtin_unreachable()
    #elif defined(_MSC_VER)
        #define UNREACHABLE() __assume(0)
    #else
        #define UNREACHABLE() static_cast<void>(0)
    #endif
#else
        // TODO : __LINE__ info
        #define UNREACHABLE() throw std::runtime_error{"UNREACHABLE() macro reached in debug mode"}
#endif

#if !defined(NDEBUG)
namespace sc::detail {
    NO_INLINE void assert_failed(char const* expression, char const* file, int line, std::string const& message) {
        using namespace std::string_literals;
        throw std::runtime_error
                { "In file "s + file + "\n"
                  "Line " + std::to_string(line) + " : Assert failed at '"s + expression + "'.\n"
                  "Message : " + message };
    }
};
#endif
