
#pragma once

#include <type_traits>
#include <iterator>


namespace sc {

    // Is valid expression

    namespace detail {
        template <template <class...> class Expression, typename SFINAE, typename... Ts>
        constexpr bool is_valid_expression = false;

        template <template <class...> class Expression, typename... Ts>
        constexpr bool is_valid_expression<Expression, std::void_t<Expression<Ts...>>, Ts...> = true;
    }
    template<template<class...> class Expression, typename... Ts>
    constexpr bool is_valid_expression = detail::is_valid_expression<Expression, void, Ts...>;

    // Is iterator

    namespace detail {
        template <class T, class SFINAE = void>
        constexpr bool is_iterator = false;

        template <class T>
        constexpr bool is_iterator<T, std::enable_if_t<
            !std::is_same_v<typename std::iterator_traits<T>::value_type, void>
        >> = true;
    }
    template <class T>
    constexpr bool is_iterator = detail::is_iterator<T>;

    // Is iterable

    namespace detail {
        template <class T, class SFINAE = void>
        constexpr bool is_iterable = false;

        template <class T>
        constexpr bool is_iterable<T, std::enable_if_t<
            is_iterator<typename T::iterator> &&
            std::is_same_v<typename T::iterator, std::remove_reference_t<decltype(std::declval<T>().begin())>> &&
            std::is_same_v<typename T::iterator, std::remove_reference_t<decltype(std::declval<T>().end())>>
        >> = true;
    }
    template <class T>
    constexpr bool is_iterable = detail::is_iterable<T>;

    // Can emplace in

    namespace detail {
        template<class Container, class SFINAE = void>
        struct emplace_value {
            static constexpr bool value = false;
        };

#define HAS_METHOD(name) \
        template <class Container, class SFINAE = void> \
        constexpr bool has_##name = false; \
        template <class Container> \
        constexpr bool has_##name<Container, std::void_t< \
            decltype(std::declval<Container>().name()) \
        >> = true;

        HAS_METHOD(emplace_back);
        HAS_METHOD(emplace_front);
        HAS_METHOD(emplace);

#undef HAS_METHOD

        template<class Container>
        struct emplace_value<Container, std::enable_if_t<
                has_emplace_back<Container>
        >> {
            static constexpr bool value = true;

            template<class...Args>
            static void invoke(Container &c, Args &&...args) {
                c.emplace_back(std::forward<Args>(args)...);
            }
        };

        template<class Container>
        using emplace_front_expr = decltype(std::declval<Container>().emplace_front());

        template<class Container>
        struct emplace_value<Container, std::enable_if_t<
                !has_emplace_back<Container> &&
                has_emplace_front<Container>
        >> {
            static constexpr bool value = true;

            template<class...Args>
            static void invoke(Container &c, Args &&...args) {
                c.emplace_front(std::forward<Args>(args)...);
            }
        };

        template<class Container>
        using emplace_expr = decltype(std::declval<Container>().emplace());

        template<class Container>
        struct emplace_value<Container, std::enable_if_t<
                !has_emplace_back<Container> &&
                !has_emplace_front<Container> &&
                has_emplace<Container>
        >> {
            static constexpr bool value = true;

            template<class...Args>
            static void invoke(Container &c, Args &&...args) {
                c.emplace(std::forward<Args>(args)...);
            }
        };
    }
    template <class Container>
    constexpr bool can_emplace_in = detail::emplace_value<Container>::value;

    template <class Container, class...Args>
    void emplace_in(Container& container, Args&&...args) {
        detail::emplace_value<Container>::invoke(container, std::forward<Args>(args)...);
    }

};
