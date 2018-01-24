
#pragma once

#include <type_traits>
#include <iterator>


namespace sc {

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
    // TODO : Make it work for built-in arrays

    namespace detail {
        template <class T, class SFINAE = void>
        constexpr bool is_iterable = false;

        template <class T>
        constexpr bool is_iterable<T, std::enable_if_t<
            std::is_same_v<
                std::remove_reference_t<decltype(std::begin(std::declval<T>()))>,
                std::remove_reference_t<decltype(std::end  (std::declval<T>()))>
            >
        >> = true;
    }
    template <class T>
    constexpr bool is_iterable = detail::is_iterable<T>;

    namespace detail {
        template <class T, class SFINAE = void>
        constexpr bool has_continuous_storage = false;

        template <class T>
        constexpr bool has_continuous_storage<T, std::enable_if_t<
            std::is_integral_v<decltype(std::declval<T>().size())> &&
            std::is_pointer_v<decltype(std::declval<T>().data())>
        >> = true;
    }
    template <class T>
    constexpr bool has_continuous_storage = detail::has_continuous_storage<T>;

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

        HAS_METHOD(emplace_back)
        HAS_METHOD(emplace_front)
        HAS_METHOD(emplace)

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

    // Efficient argument type

    namespace detail {
        template <class T, class SFINAE = void>
        struct efficient_argument {
            using type = T const&;
        };

        template <class T>
        struct efficient_argument<T, std::enable_if_t<
            (sizeof(T) <= 2 * sizeof(uintptr_t)) &&
            std::is_trivially_copy_constructible_v<T>
        >> {
            using type = T;
        };
    }
    template <class T>
    using efficient_argument_t = typename detail::efficient_argument<T>::type;

};
