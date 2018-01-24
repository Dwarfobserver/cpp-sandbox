
#pragma once

#include <type_traits>
#include <cstddef>
#include <cstdint>
#include <iostream>


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
        std::byte const* begin;
        std::byte const* end;
    };
    struct binary_ospan {
        std::byte* begin;
        std::byte* end;
    };
    struct binary_span {
        std::byte* begin;
        std::byte* end;
    };

    /// Input operators

    template <class T> binary_ospan& operator<<(binary_ospan& span, T const& data);
    template <class T> binary_span&  operator<<(binary_span& span,  T const& data);

    /// Output operators

    template <class T> binary_ispan& operator>>(binary_ispan& span, T& data);
    template <class T> binary_span&  operator>>(binary_span& span,  T& data);

    /// Get the serialized size

    template <class T> int serialized_size(T const& data = T());

    /// Traits

    template <class T> constexpr bool is_serializable = false;
    template <class T> constexpr bool is_trivially_serializable = false;

    /// Implementation

    // Spans types used to recognize operation type.

    namespace detail::span {
        struct input {
            std::byte const* begin;
            std::byte const* end;
        };
        struct output {
            std::byte* begin;
            std::byte* end;
        };
        struct accumulator {
            int value;
        };
    }

    // Operation type

    namespace detail::span {
        template <class Span, class T, class SFINAE = void>
        struct operation;
    }

    // The operator '&' with no overload nor specialization.
    // Picked after used-defined operators on serializable types.

    namespace detail::span {
        template <class Span, class T>
        auto& operator&(Span& span, T& data) {
            operation<Span, T>::invoke(span, data);
            return span;
        };
    }

    // Public operations implementation

    // Input

    template <class T>
    binary_ispan& operator>>(binary_ispan& span, T& data) {
        reinterpret_cast<detail::span::input&>(span) & data;
        return span;
    };
    template <class T>
    binary_span& operator>>(binary_span& span, T& data) {
        reinterpret_cast<detail::span::input&>(span) & data;
        return span;
    };

    // Output

    template <class T>
    binary_ospan& operator<<(binary_ospan& span, T const& data) {
        reinterpret_cast<detail::span::output&>(span) & const_cast<T&>(data);
        return span;
    };
    template <class T>
    binary_span& operator<<(binary_span& span, T const& data) {
        reinterpret_cast<detail::span::output&>(span) & const_cast<T&>(data);
        return span;
    };

    // Runtime size

    template <class T>
    int serialized_size(T const& data) {
        detail::span::accumulator accumulator{0};
        accumulator & const_cast<T&>(data);
        return accumulator.value;
    };

    // Trivial types specializations

    template <> constexpr bool is_trivially_serializable<int8_t>  = true;
    template <> constexpr bool is_trivially_serializable<int16_t> = true;
    template <> constexpr bool is_trivially_serializable<int32_t> = true;
    template <> constexpr bool is_trivially_serializable<int64_t> = true;

    template <> constexpr bool is_trivially_serializable<uint8_t>  = true;
    template <> constexpr bool is_trivially_serializable<uint16_t> = true;
    template <> constexpr bool is_trivially_serializable<uint32_t> = true;
    template <> constexpr bool is_trivially_serializable<uint64_t> = true;

    template <> constexpr bool is_trivially_serializable<bool>   = true;
    template <> constexpr bool is_trivially_serializable<float>  = true;
    template <> constexpr bool is_trivially_serializable<double> = true;

    // Trivial types expressions

    namespace detail::span {
        template <class T>
        struct operation<input, T, std::enable_if_t<
                is_trivially_serializable<T>
        >> {
            static void invoke(input& span, T& data) {
                data = *reinterpret_cast<T const*>(span.begin);
                span.begin += serialized_size<T>();
            }
        };
        template <class T>
        struct operation<output, T, std::enable_if_t<
                is_trivially_serializable<T>
        >> {
            static void invoke(output& span, T& data) {
                *reinterpret_cast<T*>(span.begin) = data;
                span.begin += serialized_size<T>();
            }
        };
        template <class T>
        struct operation<accumulator, T, std::enable_if_t<
                is_trivially_serializable<T>
        >> {
            static void invoke(accumulator& span, T&) { span.value += sizeof(T); }
        };
    }

    // TODO Built-in arrays
/*
    template <class T, int N>
    constexpr bool is_serializable<T(&)[N]> = is_serializable<T>;

    namespace detail::span {
        template <class T, int N>
        struct operation<input, T(&)[N], std::enable_if_t<
                is_serializable<T(&)[N]>
        >> {
            static void invoke(input& span, T& data) {
                std::copy(span)
                data = *reinterpret_cast<T const*>(span.begin);
                span.begin += serialized_size<T>();
            }
        };
        template <class T, int N>
        struct operation<output, T(&)[N], std::enable_if_t<
                is_serializable<T(&)[N]>
        >> {
            static void invoke(output& span, T& data) {
                *reinterpret_cast<T*>(span.begin) = data;
                span.begin += serialized_size<T>();
            }
        };
        template <class T, int N>
        struct operation<accumulator, T(&)[N], std::enable_if_t<
                is_serializable<T(&)[N]>
        >> {
            static void invoke(accumulator& span, T&) { span.value += sizeof(T); }
        };
    }
*/
}
