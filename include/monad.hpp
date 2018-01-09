
#pragma once


#include <functional>

namespace sc {

    template <template <class> class Monad>
    struct monad_traits;

/** @_MONAD_CONCEPT

    template <template <class> class Monad>
    struct monad_traits {
        template <class T>
        static Monad<T> join(Monad<Monad<T>> const& m);

        template <class T, class F>
        static auto fmap(Monad<T> const& m, F const& f) -> Monad<decltype(f(std::declval<T>()))>;

    };
*/

    template <>
    struct monad_traits<std::optional> {
        template <class T>
        static std::optional<T> join(std::optional<std::optional<T>> const& m) {
            if (m) return *m;
            else return {};
        }

        template <class T, class F>
        static auto fmap(std::optional<T> const& m, F const& f) -> std::optional<decltype(f(*m))> {
            if (m) return f(*m);
            else return {};
        }
    };
/*
    template <class Container, class T>
    struct monad_traits<std::enable_if_t< // TODO Iterable & Insertable trait, and generic (optimal) inserter
            true
    , Container>, T> {
        template <class F>
        static auto fmap(Container<T> const& m, F const& f) -> Container<decltype(f(std::declval<T>()))> {
            Container<decltype(F(*m))> res;
            for (auto const& val : m) {
                res.insert(res.end(), f(val));
            }
            return res;
        }
        static Container<T> join(Container<Container<T>> const& m) {
            Container<T> res;
            for (auto const& cont : m) {
                for (auto const& val : cont) {
                    res.insert(res.end(), val);
                }
            }
            return res;
        }
    };
*/
    namespace monad_operator {

        template <template <class> class Monad, class T, class F>
        auto operator|(Monad<T> const &m, F const &f) {
            auto res = monad_traits<Monad>::fmap(m, f);
            return     monad_traits<Monad>::join(res);
        }
/*
        template <class T, class F>
        auto operator|(T const& val, F const& f) {
            // Need Monad from Monad<U>
        };*/
    }

}
