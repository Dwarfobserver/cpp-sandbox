
#pragma once

#include <type_traits>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <type_traits.hpp>


namespace sc {

    /// Make a class serializable :
    /// Implement the operator& with a template class and this signature :
    ///
    /// template <class Span>
    /// constexpr auto& operator&(Span& span, MyClass& data);
    ///
    /// Look in 'tests_serializer_span.cpp' for working examples.

    // Available spans

    struct binary_ispan {
        std::byte const *begin;
        std::byte const *end;
    };
    struct binary_ospan {
        std::byte *begin;
        std::byte *end;
    };
    struct binary_span {
        std::byte *begin;
        std::byte *end;
    };

    /// Input operators

    template<class T>
    binary_ospan &operator<<(binary_ospan &span, T const &data);

    template<class T>
    binary_span &operator<<(binary_span &span, T const &data);

    /// Output operators

    template<class T>
    binary_ispan &operator>>(binary_ispan &span, T &data);

    template<class T>
    binary_span &operator>>(binary_span &span, T &data);

    /// Get the serialized size

    template<class T>
    constexpr int serialized_size();
/*
    template<class T>
    int serialized_size(T const &data);
*/
    /// Traits TODO

    template<class T, class SFINAE = void> constexpr bool is_serializable = false;
    template<class T, class SFINAE = void> constexpr bool is_trivially_serializable = false;

    /// Implementation

    // Spans types used to recognize operation type.

    namespace detail::span {
        struct input {
            std::byte const *begin;
            std::byte const *end;
        };
        struct output {
            std::byte *begin;
            std::byte *end;
        };
        struct accumulator {
            int value;
        };
        template<size_t N>
        struct constexpr_accumulator {
            using is_constexpr_accumulator = std::true_type;
            static constexpr size_t value = N;
            static constexpr constexpr_accumulator instance{};
        };
        struct constexpr_accumulator_tag {};
    }

    // Operation type. Usage : op<Span>::of<Span, T>::invoke(span, data);

    namespace detail::span {
        template<class Span, class T, class SFINAE = void>
        struct operation {
            static constexpr std::enable_if_t<
                std::is_same_v<typename Span::is_constexpr_accumulator, std::true_type>,
            constexpr_accumulator<Span::value + operation<constexpr_accumulator_tag, T>::invoke()>
            > const&
            invoke(Span &, T &) {
                using serialization_assert = typename Span::is_constexpr_accumulator; // Fail means T is not serializable
                constexpr auto size = Span::value + operation<constexpr_accumulator_tag, T>::invoke();
                return constexpr_accumulator<size>::instance;
            }
        };
    }

    // The operator '&' with no overload nor specialization.
    // Picked after used-defined operators on serializable types.

    namespace detail::span {
        template<class Span, class T>
        auto &operator&(Span &span, T &data) {
            return operation<Span, T>::invoke(span, data);
        };
    }

    // Public operations implementation

    // Input

    template<class T>
    binary_ispan &operator>>(binary_ispan &span, T &data) {
        reinterpret_cast<detail::span::input &>(span) & data;
        return span;
    };
    template<class T>
    binary_span &operator>>(binary_span &span, T &data) {
        reinterpret_cast<detail::span::input &>(span) & data;
        return span;
    };

    // Output

    template<class T>
    binary_ospan &operator<<(binary_ospan &span, T const &data) {
        reinterpret_cast<detail::span::output &>(span) & const_cast<T &>(data);
        return span;
    };
    template<class T>
    binary_span &operator<<(binary_span &span, T const &data) {
        reinterpret_cast<detail::span::output &>(span) & const_cast<T &>(data);
        return span;
    };

    // Runtime size

    template<class T>
    int serialized_size(T const &data) {
        detail::span::accumulator accumulator{0};
        accumulator & const_cast<T &>(data);
        return accumulator.value;
    };

    // Compile-time size

    template<class T>
    constexpr int serialized_size() {
        using acccumulator_t = typename detail::span::constexpr_accumulator<0>;
        using result_t = typename std::remove_reference_t<decltype(
            std::declval<acccumulator_t &>() & std::declval<T &>()
        )>;
        return result_t::value;
    }

    // Detect if T is serializable
/*
    template <class T>
    constexpr bool is_serializable<T, std::enable_if_t<
            std::is_same_v<binary_span&, decltype(std::declval<binary_span&>() << std::declval<T const&>)> &&
            std::is_same_v<binary_span&, decltype(std::declval<binary_span&>() >> std::declval<T&>)> &&
            std::is_same_v<int,          decltype(serialized_size(std::declval<T const&>))>
    >> = true;
*/
    // Trivial types specializations

    template<> constexpr bool is_trivially_serializable<int8_t> =  true;
    template<> constexpr bool is_trivially_serializable<int16_t> = true;
    template<> constexpr bool is_trivially_serializable<int32_t> = true;
    template<> constexpr bool is_trivially_serializable<int64_t> = true;

    template<> constexpr bool is_trivially_serializable<uint8_t> =  true;
    template<> constexpr bool is_trivially_serializable<uint16_t> = true;
    template<> constexpr bool is_trivially_serializable<uint32_t> = true;
    template<> constexpr bool is_trivially_serializable<uint64_t> = true;

    template<> constexpr bool is_trivially_serializable<char> = true;
    template<> constexpr bool is_trivially_serializable<bool> = true;
    template<> constexpr bool is_trivially_serializable<float> = true;
    template<> constexpr bool is_trivially_serializable<double> = true;
    template<> constexpr bool is_trivially_serializable<std::byte> = true;

    // Trivial types operations

    namespace detail::span {

        template<class T>
        struct operation<input, T, std::enable_if_t<
                is_trivially_serializable<T>
        >> {
            static input &invoke(input &span, T &data) {
                data = *reinterpret_cast<T const *>(span.begin);
                span.begin += serialized_size<T>();
                return span;
            }
        };

        template<class T>
        struct operation<output, T, std::enable_if_t<
                is_trivially_serializable<T>
        >> {
            static output &invoke(output &span, T &data) {
                *reinterpret_cast<T *>(span.begin) = data;
                span.begin += serialized_size<T>();
                return span;
            }
        };

        template<class T>
        struct operation<accumulator, T, std::enable_if_t<
                is_trivially_serializable<T>
        >> {
            static accumulator &invoke(accumulator &acc, T &) {
                acc.value += serialized_size<T>();
                return acc;
            }
        };

        template<class T>
        struct operation<constexpr_accumulator_tag, T, std::enable_if_t<
                is_trivially_serializable<T>
        >> {
            static constexpr int invoke() { return sizeof(T); }
        };
    }

}

// std::pair & std::tuple operations (in global namespace)

template <class Span, class T1, class T2>
auto& operator&(Span& span, std::pair<T1, T2>& pair) {
    return span & pair.first & pair.second;
};

namespace sc::detail::span {
    template <class Span, class Int, class...Ts>
    struct tuple_explore {
        static auto& invoke(Span& span, std::tuple<Ts...>& tuple) {
            using next_indice_t = typename std::integral_constant<int, Int::value - 1>;
            using operation_t   = typename span::tuple_explore<Span, next_indice_t, Ts...>;

            return operation_t::invoke(span, tuple) & std::get<Int::value>(tuple);
        }
    };
    template <class Span, class...Ts>
    struct tuple_explore<Span, std::integral_constant<int, 0>, Ts...> {
        static auto& invoke(Span& span, std::tuple<Ts...>& tuple) {
            return span & std::get<0>(tuple);
        }
    };
}

template <class Span, class...Ts>
auto& operator&(Span& span, std::tuple<Ts...>& tuple) {
    using last_indice_t = typename std::integral_constant<int, sizeof...(Ts) - 1>;
    using operation_t   = typename sc::detail::span::tuple_explore<Span, last_indice_t, Ts...>;

    return operation_t::invoke(span, tuple);
};

namespace sc {

    // Iterables operations
/*
    namespace detail::span {
        template <class T>
        struct operation<input, T, std::enable_if_t<
                is_iterable<T>// && is_serializable<std::iterator_value_of<T>> && size
        >> {
            static input& invoke(input& span, T& data) {
                uint32_t size;
                span & size;
                data.resize(size);
                for (auto& val : data) span & val;
                return span;
            }
        };
        template <class T>
        struct operation<output, T, std::enable_if_t<
                is_iterable<T>// && is_serializable<std::iterator_value_of<T>> && resize
        >> {
            static output& invoke(output& span, T& data) {
                uint32_t size = data.size();
                span & size;
                for (auto& val : data) span & val;
                return span;
            }
        };
        template <class T>
        struct operation<accumulator, T, std::enable_if_t< // TODO Specialization for T has constexpr size
                is_iterable<T>// && is_serializable<std::iterator_value_of<T>>
        >> {
            static accumulator& invoke(accumulator& acc, T& data) {
                acc.value += serialized_size<uint32_t>();
                for (auto& val : data) acc.value += serialized_size(val);
                return acc;
            }
        };
    }
*/
}
