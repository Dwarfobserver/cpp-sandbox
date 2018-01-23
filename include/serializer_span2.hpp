
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

    /// Get the serialized size at compile-time or runtime

    template <class T> constexpr int serialized_size();
    template <class T> int serialized_size(T const& data);
    
    /// Compile-time size detection

    template <class T, class SFINAE = void>
    constexpr bool has_constexpr_serialized_size = false;

    /// Implementation

    // Resolver function

    namespace detail {
        template<class Span, class T, class SFINAE = void>
        struct span_operation;

        template <class Span, class T>
        constexpr auto& operator&(Span& span, T& data) {
            return span_operation<Span, T>::invoke(span, data);
        };
    }

    // Operation types

    namespace detail {
        template<class T, class SFINAE = void>
        struct input_operation;
        template<class T, class SFINAE = void>
        struct output_operation;
        template<class T, class SFINAE = void>
        struct constexpr_size_operation;
        template<class T, class SFINAE = void>
        struct size_operation {
            static constexpr int invoke(T const&) {
                static_assert(has_constexpr_serialized_size<T>,
                              "[T] do not have compile-time or runtime size operation.");
                return serialized_size<T>();
            }
        };
    }

    // Span types

    namespace detail {
        struct input_span {
            std::byte const *begin;
            std::byte const *end;
        };
        struct output_span {
            std::byte *begin;
            std::byte *end;
        };
        struct size_accumulator {
            int value;
        };
        template <int N>
        struct constexpr_size_accumulator {
            using is_constexpr_size_accumulator = std::true_type;
            static constexpr constexpr_size_accumulator instance{};
            static constexpr int value = N;
        };
    }

    // Compile-time size detection

    template <class T>
    constexpr bool has_constexpr_serialized_size<T, std::enable_if_t<
        std::is_integral_v<decltype(
            std::declval<detail::constexpr_size_accumulator<0>&>() & std::declval<T&>()
        )>
    >> = true;

    // 'span_operation' redirection

    namespace detail {
        template <class T>
        struct span_operation<input_span, T, void> {
            static input_span& invoke(input_span& span, T& data) {
                input_operation<T>::invoke(span, data);
                return span;
            }
        };
        template <class T>
        struct span_operation<output_span, T, void> {
            static output_span& invoke(output_span& span, T& data) {
                output_operation<T>::invoke(span, data);
                return span;
            }
        };
        template <class T>
        struct span_operation<size_accumulator, T, void> {
            static size_accumulator& invoke(size_accumulator& acc, T& data) {
                acc.value += size_operation<T>::invoke(data);
                return acc;
            }
        };
        template <class Acc, class T>
        struct span_operation<Acc, T, std::enable_if_t<
            std::is_same_v<Acc::is_constexpr_size_accumulator, std::true_type>
        >> {
            static constexpr auto& invoke(Acc&, T&) {
                return constexpr_size_accumulator<
                    Acc::value + constexpr_size_operation<T>::invoke()
                >::instance;
            }
        };
    }

    // Public operators implementation

    // '<<' redirection

    template <class T>
    binary_ospan& operator<<(binary_ospan& span, T const& data) {
        return reinterpret_cast<binary_ospan&>(
                reinterpret_cast<detail::output_span&>(span) & const_cast<T&>(data));
    };
    template <class T>
    binary_span& operator<<(binary_span& span, T const& data) {
        return reinterpret_cast<binary_span&>(
                reinterpret_cast<detail::output_span&>(span) & const_cast<T&>(data));
    };

    // '>>' redirection

    template <class T>
    binary_ispan& operator>>(binary_ispan& span, T& data) {
        return reinterpret_cast<binary_ispan&>(
                reinterpret_cast<detail::input_span&>(span) & data);
    };
    template <class T>
    binary_span& operator>>(binary_span& span, T& data) {
        return reinterpret_cast<binary_span&>(
                reinterpret_cast<detail::input_span&>(span) & data);
    };

    // Runtime size redirection

    template <class T>
    int serialized_size(T const& data) {
        detail::size_accumulator acc{0};
        return (acc & data).value;
    }

    // Compile-time size redirection

    template <class T>
    constexpr int serialized_size() {
        static_assert(has_constexpr_serialized_size<T>,
                      "'T' do not have compile-time size operation.");

        using accumulator_t = typename std::remove_reference_t<decltype(
        std::declval<detail::constexpr_size_accumulator<0>&>() & std::declval<T&>()
        )>;
        return accumulator_t::value;
    }

    // Trivial types operations

    template <class T>
    constexpr bool is_trivially_serializable = false;

    namespace detail {
        template <class T>
        struct constexpr_size_operation<T, std::enable_if_t<
            is_trivially_serializable<T>
        >> {
            static constexpr int invoke() { return sizeof(T); }
        };
        template <class T>
        struct input_operation<T, std::enable_if_t<
                is_trivially_serializable<T>
        >> {
            static constexpr void invoke(input_span& span, T& data) {
                data = *reinterpret_cast<T const*>(span.begin);
                span.begin += serialized_size<T>();
            }
        };
        template <class T>
        struct output_operation<T, std::enable_if_t<
                is_trivially_serializable<T>
        >> {
            static constexpr void invoke(output_span& span, T const& data) {
                *reinterpret_cast<T*>(span.begin) = data;
                span.begin += serialized_size<T>();
            }
        };
    }

    // Trivial types specializations

    template <> constexpr bool is_trivially_serializable<bool> = true;

    template <> constexpr bool is_trivially_serializable<int8_t> = true;
    template <> constexpr bool is_trivially_serializable<int16_t> = true;
    template <> constexpr bool is_trivially_serializable<int32_t> = true;
    template <> constexpr bool is_trivially_serializable<int64_t> = true;

    template <> constexpr bool is_trivially_serializable<uint8_t> = true;
    template <> constexpr bool is_trivially_serializable<uint16_t> = true;
    template <> constexpr bool is_trivially_serializable<uint32_t> = true;
    template <> constexpr bool is_trivially_serializable<uint64_t> = true;

}
