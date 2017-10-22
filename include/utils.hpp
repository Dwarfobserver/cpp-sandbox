
#pragma once

#include <optional>


/// Defines SC_CACHE_LINE_SIZE to use structures based on custom cache line size.
// TODO Get std::hardware_destructive_interference_size from a modern compiler
#ifndef SC_CACHE_LINE_SIZE
#define SC_CACHE_LINE_SIZE 64
#endif


namespace sc {

    template <class F>
    struct defer : F {
        ~defer() {
            static_assert(noexcept(std::declval<F>()()), "The deferred function must be marked noexcept.");

            F::operator()();
        }
    };
    template<class F> defer(F) -> defer<F>;

    namespace optional_monad {
        namespace detail {
            template <class T, class Expr = void>
            struct add_optionality {
                using type = std::optional<T>;
            };
            template <class T>
            struct add_optionality<std::optional<T>> {
                using type = std::optional<T>;
            };

            template <class T>
            using add_optionality_t = typename add_optionality<T>::type;
        }

        template <class T, class F>
        auto operator|(std::optional<T> opt, F&& f) -> detail::add_optionality_t<decltype(f(*opt))> {
            if (opt) {
                return f(*opt);
            }
            else return {};
        }
    }
}
