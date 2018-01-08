
#pragma once

#include <utility>
#include <sstream>


namespace sc {


    namespace traits {

        namespace detail {
            template <template <class...> class Expression, typename SFINAE, typename... Ts>
            constexpr bool is_valid = false;

            template <template <class...> class Expression, typename... Ts>
            constexpr bool is_valid<Expression, std::void_t<Expression<Ts...>>, Ts...> = true;
        }

        template<template<class...> class Expression, typename... Ts>
        constexpr bool is_valid = detail::is_valid<Expression, void, Ts...>;
    }

    namespace detail {

        template<class T>
        using std_to_string_expr = decltype(std::to_string(std::declval<T>()));

        template<class T>
        using to_string_expr = decltype(to_string(std::declval<T>()));

        template<typename T>
        using sstream_expr = decltype(std::declval<std::ostringstream&>() << std::declval<T>());

        template<typename T>
        using begin_expr = decltype(std::declval<T>().begin());

        template<typename T>
        using end_expr = decltype(std::declval<T>().end());

    }

    namespace traits {

        template <class T>
        using iterator_value_of = std::decay_t<decltype(*std::declval<sc::detail::begin_expr<T>>())>;

        template <class T>
        constexpr bool has_to_string = traits::is_valid<sc::detail::to_string_expr, T>;

        template <class T>
        constexpr bool has_std_to_string = traits::is_valid<sc::detail::std_to_string_expr, T>;

        template <class T>
        constexpr bool is_string_streamable = traits::is_valid<sc::detail::sstream_expr, T>;

        template <class T>
        constexpr bool is_iterable = traits::is_valid<sc::detail::begin_expr, T> &&
                                     traits::is_valid<sc::detail::end_expr, T>;

        template <class T>
        constexpr bool can_make_string = // TODO recursive test doesn't seems to stop at first condition test
            std::is_constructible_v<std::string, T> ||
            traits::has_to_string<T> ||
            traits::has_std_to_string<T> ||
            traits::is_string_streamable<T> ||
            (traits::is_iterable<T> && can_make_string<traits::iterator_value_of<T>>);
    }

    template <class T>
    std::string make_string(T const& val);

    template <class T1, class T2>
    std::string make_string(std::pair<T1, T2> const& pair);

    template <class...Ts>
    std::string make_string(std::tuple<Ts...> const& tuple);

    inline std::string make_string(bool val) {
        return val ? "true" : "false";
    }
    inline std::string make_string(char val) {
        std::string str;
        str.push_back(val);
        return str;
    }

    // Pair

    template <class T1, class T2>
    std::string make_string(std::pair<T1, T2> const& pair) {
        return "{ " + sc::make_string(pair.first) + ", " + sc::make_string(pair.second) + " }";
    };

    // Tuple

    namespace detail {

        template <int N>
        struct integral_t { enum { value = N }; };

        template <class Int, class...Ts>
        struct tuple_to_string_rec_t;

        template <class Int, class...Ts>
        struct tuple_to_string_rec_t {
            static void concat(std::tuple<Ts...> const& tuple, std::string& str) {
                str += ", " + sc::make_string(std::get<Int::value>(tuple));
                tuple_to_string_rec_t<integral_t<Int::value + 1>, Ts...>::concat(tuple, str);
            };
        };

        template <class...Ts>
        struct tuple_to_string_rec_t<integral_t<sizeof...(Ts)>, Ts...> {
            static void concat(std::tuple<Ts...> const& tuple, std::string& str) {};
        };

        template <class...Ts>
        struct tuple_to_string_t {
            static void concat(std::tuple<Ts...> const& tuple, std::string& str) {
                str += sc::make_string(std::get<0>(tuple));
                tuple_to_string_rec_t<integral_t<1>, Ts...>::concat(tuple, str);
            };
        };

    }

    template <class T>
    std::string make_string(std::tuple<T> const& tuple) {
        return "{ " + sc::make_string(std::get<0>(tuple)) + " }";
    }

    template <class...Ts>
    std::string make_string(std::tuple<Ts...> const& tuple) {
        std::string str = "{ ";
        detail::tuple_to_string_t<Ts...>::concat(tuple, str);
        str += " }";
        return str;
    }

    // Generic call

    namespace detail {

        template <class T, class SFINAE = void>
        struct make_string_t {};

        template <class T>
        struct make_string_t<T, std::enable_if_t<
                std::is_constructible_v<std::string, T>
        >> {
            static std::string make_string(T const& val) {
                return std::string{ val };
            }
        };

        template <class T>
        struct make_string_t<T, std::enable_if_t<
                !std::is_constructible_v<std::string, T> &&
                traits::has_to_string<T>
        >> {
            static std::string make_string(T const& val) {
                return to_string(val);
            }
        };

        template <class T>
        struct make_string_t<T, std::enable_if_t<
                !std::is_constructible_v<std::string, T> &&
                !traits::has_to_string<T> &
                traits::has_std_to_string<T>
        >> {
            static std::string make_string(T const& val) {
                return std::to_string(val);
            }
        };

        template <class T>
        struct make_string_t<T, std::enable_if_t<
                !std::is_constructible_v<std::string, T> &&
                !traits::has_to_string<T> &
                !traits::has_std_to_string<T> &&
                traits::is_string_streamable<T>
        >> {
            static std::string make_string(T const& val) {
                std::ostringstream ss;
                ss << val;
                return ss.str();
            }
        };

        template <class T>
        struct make_string_t<T, std::enable_if_t<
                !std::is_constructible_v<std::string, T> &&
                !traits::has_to_string<T> &
                !traits::has_std_to_string<T> &&
                !traits::is_string_streamable<T> &&
                traits::is_iterable<T>//&& traits::can_make_string<traits::iterator_value_of<T>>
        >> {
            static std::string make_string(T const& val) {
                using value_type = traits::iterator_value_of<T>;

                std::string str = "[ ";
                auto begin = val.begin();
                const auto end = val.end();
                if (begin != end) {
                    str += sc::make_string(*begin);
                    ++begin;
                    while (begin != end) {
                        str += ", " + sc::make_string(*begin);
                        ++begin;
                    }
                }
                str += " ]";
                return str;
            }
        };
    }

    template <class T>
    std::string make_string(T const& val) {
        return detail::make_string_t<T>::make_string(val);
    }

}

