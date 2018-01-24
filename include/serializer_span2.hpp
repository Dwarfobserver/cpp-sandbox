
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

    // Spans for implementation
    // Used to recognize operation categories (and compute them).

    namespace detail {
        struct span_input {
            std::byte const* begin;
            std::byte const* end;
        };
        struct span_output {
            std::byte* begin;
            std::byte* end;
        };
        struct span_size {
            int value;
        };
        struct span_constexpr_size {};
    }

    // Operation type
    // Used by expression templates to compute the result.

    namespace detail::span {
        template <class Span, class T, class SFINAE = void>
        struct operation;
    }

    // Expression generator
    // Used by the operator '&' to build expression templates.

    namespace detail::span {
        template <class Expression, class T, class SFINAE = void>
        struct expression_generator;
    }

    // Expression templates
    // Built at compilation to compute the result recursively.
    // They were needed to match the 'constexpr' signature of the operator '&'.

    namespace detail::span {
        template <class Span>
        struct expression {
            using span_type = Span;

            constexpr explicit expression(Span& span) :
                    span_(span) {}

            void invoke() const { }
            constexpr Span& span() { return span_; }

            Span& span_;
        };

        template <class Expression, class T>
        struct expression_rec {
            using span_type = typename Expression::span_type;

            constexpr expression_rec(Expression& expression, T& data) :
                    expression_(expression), data_(data) {}

            void invoke() const {
                operation<span_type, T>::invoke(span(), data_);
                expression_.invoke();
            }
            constexpr span_type& span() { return expression_.span(); }

            Expression& expression_;
            T& data_;
        };

        // The constexpr size have a different signature (less parameters and constexpr result).

        struct constexpr_size_expression {
            using span_type = span_constexpr_size;
            static constexpr int value = 0;
        };
        template <class Expression, class T>
        struct constexpr_size_expression_rec {
            using span_type = span_constexpr_size;
            static constexpr int value = operation<span_type, T>::invoke() + Expression::value;
        };
    }

    // Expression generators implementation (for the 4 operations category).

    namespace detail::span {

        template <class Expression, class T>
        struct expression_generator<Expression, T, void/*std::enable_if_t<
                std::is_same_v<Expression::span_type, span_input>  ||
                std::is_same_v<Expression::span_type, span_output> ||
                std::is_same_v<Expression::span_type, span_size>
        >*/> {
            static constexpr auto invoke(Expression &expression, T &data)
                    -> expression_rec<Expression, T> {
                return { expression, data };
            }
        };

        template <class Expression, class T>
        struct expression_generator<Expression, T, std::enable_if_t<
                std::is_same_v<Expression::span_type, span_constexpr_size>
        >> {
            static constexpr auto invoke(Expression&, T&) {
                return constexpr_size_expression_rec<Expression, T>();
            }
        };
    }

    // The operator '&' with no overload nor specialization, returning an expression template.
    // Picked after used-defined operators on serializable types.

    namespace detail::span {
        template <class Expression, class T>
        constexpr auto operator&(Expression& expression, T& data) {
            return expression_generator<Expression, T>::invoke(expression, data);
        };
    }

    // Public operations implementation

    // Common implementation to the 3 runtime operations.

    namespace detail::span {
        template <class Span, class T>
        void invoke_operation(Span& span, T& data) {
            constexpr auto expr = expression<Span>{ span };
            constexpr auto expr_rec = expr & data;
            expr_rec.invoke();
        };
    }

    // Input

    template <class T>
    binary_ispan& operator>>(binary_ispan& span, T& data) {
        auto& span_input = reinterpret_cast<detail::span_input&>(span);
        detail::span::invoke_operation(span_input, data);
        return span;
    };
    template <class T>
    binary_span& operator>>(binary_span& span, T& data) {
        auto& span_input = reinterpret_cast<detail::span_input&>(span);
        detail::span::invoke_operation(span_input, data);
        return span;
    };

    // Output

    template <class T>
    binary_ospan& operator<<(binary_ospan& span, T const& data) {
        auto& span_output = reinterpret_cast<detail::span_output&>(span);
        detail::span::invoke_operation(span_output, const_cast<T&>(data));
        return span;
    };
    template <class T>
    binary_span& operator<<(binary_span& span, T const& data) {
        auto& span_output = reinterpret_cast<detail::span_output&>(span);
        detail::span::invoke_operation(span_output, const_cast<T&>(data));
        return span;
    };

    // Runtime size

    template <class T>
    int serialized_size(T const& data) {
        detail::span_size accumulator{0};
        detail::span::invoke_operation(accumulator, const_cast<T&>(data));
        return accumulator.value;
    };

    // Constexpr size

    template <class T>
    constexpr int serialized_size() {
        using expression_t = std::remove_reference_t<decltype(
            std::declval<detail::span::constexpr_size_expression&>() & std::declval<T&>()
        )>;
        return expression_t::value;
    }

    // Trivial types specializations

    template <class T>
    constexpr bool is_trivially_serializable = false;

    template <> constexpr bool is_trivially_serializable<bool> = true;

    template <> constexpr bool is_trivially_serializable<int8_t> = true;
    template <> constexpr bool is_trivially_serializable<int16_t> = true;
    template <> constexpr bool is_trivially_serializable<int32_t> = true;
    template <> constexpr bool is_trivially_serializable<int64_t> = true;

    template <> constexpr bool is_trivially_serializable<uint8_t> = true;
    template <> constexpr bool is_trivially_serializable<uint16_t> = true;
    template <> constexpr bool is_trivially_serializable<uint32_t> = true;
    template <> constexpr bool is_trivially_serializable<uint64_t> = true;

    // Trivial types expressions

    namespace detail::span {
        template <class T>
        struct operation<span_input, T, std::enable_if_t<
                is_trivially_serializable<T>
        >> {
            static void invoke(span_input& span, T& data) {
                data = *reinterpret_cast<T const*>(span.begin);
                span.begin += serialized_size<T>();
            }
        };
        template <class T>
        struct operation<span_output, T, std::enable_if_t<
                is_trivially_serializable<T>
        >> {
            static void invoke(span_output& span, T& data) {
                *reinterpret_cast<T*>(span.begin) = data;
                span.begin += serialized_size<T>();
            }
        };
        template <class T>
        struct operation<span_size, T, std::enable_if_t<
                is_trivially_serializable<T>
        >> {
            static void invoke(span_size& span, T&) { span.value += sizeof(T); }
        };
        template <class T>
        struct operation<span_constexpr_size, T, std::enable_if_t<
                is_trivially_serializable<T>
        >> {
            static constexpr int invoke() { return sizeof(T); }
        };
    }


}
