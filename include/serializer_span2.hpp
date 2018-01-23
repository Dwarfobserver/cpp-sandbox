
#pragma once

#include <type_traits>
#include <cstddef>
#include <cstdint>
#include <iostream>


namespace sc {

    // TODO In rework. Goal :
    // Let user implement operator & : useful for
    // input (>>),
    // output (<<) and
    // serialized size (+).

    // Only <T> op & implemented
    // <<, >> and size redirected to the right & type

    // Spans used

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

    // Span detection traits

    namespace detail {
        template <class T> constexpr bool is_ispan = false;
        template <> constexpr bool is_ispan<binary_ispan> = true;
        template <> constexpr bool is_ispan<binary_span>  = true;

        template <class T> constexpr bool is_ospan = false;
        template <> constexpr bool is_ospan<binary_ospan> = true;
        template <> constexpr bool is_ospan<binary_span>  = true;

        template <class T> constexpr bool is_span = false;
        template <> constexpr bool is_span<binary_ispan> = true;
        template <> constexpr bool is_span<binary_ospan> = true;
        template <> constexpr bool is_span<binary_span>  = true;

        template <class T> constexpr bool is_iospan = false;
        template <> constexpr bool is_iospan<binary_span> = true;
    }

    // Serialized size accumulator

    namespace detail {
        struct size_accumulator {
            int value;
        };
        struct input_span {
            std::byte const* begin;
            std::byte const* end;
        };
        struct output_span {
            std::byte* begin;
            std::byte* end;
        };

        // Trivial type operations

        size_accumulator& operator&(size_accumulator& acc, uint8_t) {
            acc.value += sizeof(uint8_t);
            return acc;
        }
        input_span& operator&(input_span& span, uint8_t& data) {
            data = *reinterpret_cast<uint8_t const*>(span.begin);
            span.begin += sizeof(uint8_t);
            return span;
        }
        output_span& operator&(output_span& span, uint8_t const& data) {
            *reinterpret_cast<uint8_t*>(span.begin) = data;
            span.begin += sizeof(uint8_t);
            return span;
        }
    }

    // '<<' redirection

    template <class T>
    binary_ospan& operator<<(binary_ospan& span, T const& data) {
        return reinterpret_cast<binary_ospan&>(reinterpret_cast<detail::output_span&>(span) & const_cast<T&>(data));
    };
    template <class T>
    binary_span& operator<<(binary_span& span, T const& data) {
        return reinterpret_cast<binary_span&>(reinterpret_cast<detail::output_span&>(span) & const_cast<T&>(data));
    };

    // '>>' redirection

    template <class T>
    binary_ispan& operator>>(binary_ispan& span, T& data) {
        return reinterpret_cast<binary_ispan&>(reinterpret_cast<detail::input_span&>(span) & data);
    };
    template <class T>
    binary_span& operator>>(binary_span& span, T& data) {
        return reinterpret_cast<binary_span&>(reinterpret_cast<detail::input_span&>(span) & data);
    };

    // 'size' redirection

    template <class T>
    int serialized_size(T const& data = T()) {
        detail::size_accumulator acc{0};
        acc & const_cast<T&>(data);
        return acc.value;
    }

    /*

    // Select '<<', '>>' or '+=' regarding the span type

    namespace detail {
        template <class Span, class T>
        struct select_io_operation;

        template <class T>
        struct select_io_operation<binary_ispan, T> {
            static binary_ispan& invoke(binary_ispan& span, T& data) { return span >> data; }
        };
        template <class T>
        struct select_io_operation<binary_ospan, T> {
            static binary_ospan& invoke(binary_ospan& span, T const& data) { return span << data; }
        };
        template <class T>
        struct select_io_operation<size_accumulator, T> {
            static constexpr size_accumulator& invoke(size_accumulator& acc, T const& data) {
                acc.value += serialized_size(data);
                return acc;
            }
        };
    }

    // Trivial types operator '&'

    template <class Span, class T>
    Span& operator&(Span& span, T& val) {
        std::cout << "derp &\n";
        return detail::select_io_operation<Span, T>::invoke(span, val);
    }
    template <class Span, class T>
    Span& operator&(Span& span, T const& val) {
        std::cout << "derp const&\n";
        return detail::select_io_operation<Span, T>::invoke(span, val);
    }

    // Choose to reinterpret span regarding it's type for '<<' and '>>'

    namespace detail {
        template <class Span, class T>
        struct select_i_operation;
        template <class T>
        struct select_i_operation<binary_ispan, T> {
            static binary_ispan& invoke(binary_ispan& span, T& data) { return span & data; }
        };
        template <class T>
        struct select_i_operation<binary_span, T> {
            static binary_span& invoke(binary_span& span, T& data) {
                return reinterpret_cast<binary_span&>(reinterpret_cast<binary_ispan&>(span) & data);
            }
        };

        template <class Span, class T>
        struct select_o_operation;
        template <class T>
        struct select_o_operation<binary_ospan, T> {
            static binary_ospan& invoke(binary_ospan& span, T const& data) { return span & data; }
        };
        template <class T>
        struct select_o_operation<binary_span, T> {
            static binary_span& invoke(binary_span& span, T const& data) {
                return reinterpret_cast<binary_span&>(reinterpret_cast<binary_ospan&>(span) & data);
            }
        };
    }

    // General types operators '<<' and '>>'

    template <class Span, class T>
    Span& operator>>(Span& span, T& data) {
        return detail::select_i_operation<Span, T>::invoke(span, data);
    };
    template <class Span, class T>
    Span& operator<<(Span& span, T const& data) {
        return detail::select_o_operation<Span, T>::invoke(span, data);
    };

    // General type serialized size

    template <class T>
    constexpr int serialized_size(T const& data = T()) {
        detail::size_accumulator acc{};
        return (acc & data).value;
    }

     */

}
