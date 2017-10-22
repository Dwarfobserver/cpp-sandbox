
#pragma once

#include <algorithm>
#include <numeric>


namespace sc {

    template<template<class> class Collection, class T>
    class fluent : public Collection<T> {
        template <class F>
        using map_t = fluent<Collection, decltype(std::declval<F&&>()(std::declval<T const&>()))>;
    public:
        using collection_type = Collection<T>;

        template <class...Args>
        explicit fluent(Args&&...args);

        explicit fluent(std::initializer_list<T> list);

        Collection<T> const& collection() const noexcept;

        template <class F>
        fluent<Collection, T> apply(F&& f) const;

        template <class F>
        map_t<F> map(F&& unaryOp) const;

        template <class F>
        fluent<Collection, T> filter(F&& predicate) const;

        template <class F>
        T reduce(F&& binaryOp) const;

        template <class F>
        T reduce(F&& binaryOp, T const& first) const;
    };

    // ______________
    // Implementation

    template<template<class> class Collection, class T> template <class...Args>
    fluent<Collection, T>::fluent(Args &&... args) : Collection<T>(std::forward<Args>(args)...) {}

    template<template<class> class Collection, class T>
    fluent<Collection, T>::fluent(std::initializer_list<T> list) : Collection<T>(list) {}

    template<template<class> class Collection, class T>
    inline Collection<T> const& fluent<Collection, T>::collection() const noexcept {
        return *static_cast<Collection<T> const * const>(this);
    }

    template<template<class> class Collection, class T> template <class F>
    fluent<Collection, T> fluent<Collection, T>::apply(F&& f) const {
        fluent<Collection, T> result(*this);
        std::for_each(
                result.begin(),
                result.end(),
                f);
        return result;
    }

    template<template<class> class Collection, class T> template <class F>
    typename fluent<Collection, T>::template map_t<F> fluent<Collection, T>::map(F&& unaryOp) const {
        map_t<F> result;
        std::transform(
                collection_type::cbegin(),
                collection_type::cend(),
                std::inserter(result, result.end()),
                unaryOp);
        return result;
    }

    template<template<class> class Collection, class T> template <class F>
    fluent<Collection, T> fluent<Collection, T>::filter(F&& predicate) const {
        fluent<Collection, T> result;
        std::copy_if(
                collection_type::cbegin(),
                collection_type::cend(),
                std::inserter(result, result.end()),
                predicate);
        return result;
    }

    template<template<class> class Collection, class T> template <class F>
    T fluent<Collection, T>::reduce(F&& binaryOp) const {
        if (collection_type::cbegin() == collection_type::cend())
            return T();
        return std::accumulate(
                ++collection_type::cbegin(),
                collection_type::cend(),
                *collection_type::cbegin(),
                binaryOp);
    }

    template<template<class> class Collection, class T> template <class F>
    T fluent<Collection, T>::reduce(F&& binaryOp, T const& first) const {
        return std::accumulate(
                collection_type::cbegin(),
                collection_type::cend(),
                first,
                binaryOp);
    }

}
