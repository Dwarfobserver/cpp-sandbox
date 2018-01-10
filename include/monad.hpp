
#pragma once

#include <functional>
#include <type_traits.hpp>


namespace sc {

/** @_MONAD_CONCEPT

    template <template <class> class Monad>
    struct monad_traits {
        template <class MonadT>
        using value_type;

        template <class T>
        static Monad<T> join(Monad<Monad<T>> const& m);

        template <class T, class F>
        static auto fmap(Monad<T> const& m, F const& f) -> Monad<decltype(f(std::declval<T>()))>;
    };
*/
    template <template <class> class Monad, class SFINAE = void>
    struct monad_traits {
        static constexpr bool invalid = true;
    };

    template <template <class> class T, class SFINAE = void>
    constexpr bool is_monad = true;
    template <template <class> class T>
    constexpr bool is_monad<T, std::enable_if_t<
            monad_traits<T>::invalid
    >> = false;

    namespace detail {

        template <template <class> class Monad, class T, class SFINAE = void>
        constexpr bool is_joinable_monad = false;

        template <template <class> class Monad, class MonadT>
        using monad_value_type = typename monad_traits<Monad>::template value_type<MonadT>;

        template <template <class> class Monad, class T>
        constexpr bool is_joinable_monad<Monad, T, std::enable_if_t<
                is_monad<Monad> &&
                !std::is_same_v<monad_value_type<Monad, monad_value_type<Monad, T>>, void>
        >> = true;
    }

    // std::optional

    template <>
    struct monad_traits<std::optional, void> {
        template <class MonadT>
        using value_type = typename MonadT::value_type;

        template <class T, class F>
        static auto fmap(std::optional<T> const& m, F const& f) -> std::optional<decltype(f(*m))> {
            if (m) return f(*m);
            else return {};
        }

        template <class T>
        static std::optional<T> join(std::optional<std::optional<T>> const& m) {
            if (m) return *m;
            else return {};
        }
    };

    // Container

    template <template <class> class Container>
    struct monad_traits<Container, std::enable_if_t<
        is_iterable<Container<float>> && // test with random type
        can_emplace_in<Container<float>>
    >> {
        template <class MonadT>
        using value_type = typename MonadT::value_type;

        template <class T, class F>
        static auto fmap(Container<T> const& m, F const& f) -> Container<decltype(f(std::declval<T>()))> {
            Container<decltype(f(std::declval<T>()))> res;
            for (auto const& val : m) {
                emplace_in(res, f(val));
            }
            return res;
        }

        template <class T>
        static Container<T> join(Container<Container<T>> const& m) {
            Container<T> res;
            for (auto const& cont : m) {
                for (auto const& val : cont) {
                    emplace_in(res, val);
                }
            }
            return res;
        }
    };

    namespace monad_operator {

        template <template <class> class Monad, class T, class F>
        auto operator|(Monad<T> const &m, F const &f) {
            static_assert(is_monad<Monad>);

            auto res =  monad_traits<Monad>::fmap(m, f);
            if constexpr (detail::is_joinable_monad<Monad, decltype(res)>)
                 return monad_traits<Monad>::join(res);
            else return res;
        }

    }

}
