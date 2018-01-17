
#pragma once

#include <type_traits>


namespace sc {

    // Goal

    struct bytes_span;
    struct bytes_ispan;
    struct bytes_ospan;

    class my_str {
    public:
        friend bytes_span& operator&(bytes_span& span, my_str& str) {
            return span & str;
        }
    private:
        char const* str;
    };
    class my_str2 {
    public:
        friend bytes_ispan& operator>>(bytes_ispan& span, my_str& str) {
            return span >> str;
        }
        friend bytes_ospan& operator<<(bytes_ospan& span, my_str const& str) {
            return span << str;
        }
    private:
        char const* str;
    };


    struct bytes_span {
        void* begin;
        void* end;
    };
    struct bytes_ispan {
        void const* begin;
        void const* end;
    };
    struct bytes_ospan {
        void* begin;
        void* end;
    };

    namespace detail {

        template <class Span, class T, class SFINAE = void>
        constexpr bool has_io = false;
        template <class Span, class T, class SFINAE>
        constexpr bool has_io<Span, T, std::enable_if_t<
            std::is_same_v<Span&, decltype(std::declval<Span&>() & std::declval<T&>())>
        >> = true;

        template <class Span, class T, class SFINAE = void>
        constexpr bool has_i = false;
        template <class Span, class T, class SFINAE>
        constexpr bool has_i<Span, T, std::enable_if_t<
            has_io<Span, T> ||
            std::is_same_v<Span&, decltype(std::declval<Span&>() >> std::declval<T&>())>
        >> = true;

        template <class Span, class T, class SFINAE = void>
        constexpr bool has_o = false;
        template <class Span, class T, class SFINAE>
        constexpr bool has_o<Span, T, std::enable_if_t<
                has_io<Span, T> ||
                std::is_same_v<Span&, decltype(std::declval<Span&>() << std::declval<T const&>())>
        >> = true;

        template <class Span, class T, class SFINAE = void>
        struct do_i;
        template <class Span, class T, class SFINAE>
        struct do_i<Span, T, std::enable_if_t<
            has_io<Span, T>
        >> {
            void invoke(Span& span, T& data) {

            }
        };
    }

    template <class Span, class T>
    std::enable_if_t<
        (std::is_same_v<Span, bytes_span> ||
        std::is_same_v<Span, bytes_ospan> ||
        std::is_same_v<Span, bytes_ispan>)
            &&
        (true)
    ,Span&> operator<<(Span& span, T const& data) {
        if ()
    };

    namespace detail {

    }

}
