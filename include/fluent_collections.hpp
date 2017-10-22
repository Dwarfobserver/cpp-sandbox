
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
    fluent<Collection, T>::fluent(Args &&... args) : Collection<T>(std::forward<T>(args)...) {}

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
                collection_type::begin(),
                collection_type::end(),
                std::inserter(result, result.end()),
                unaryOp);
        return result;
    }

    template<template<class> class Collection, class T> template <class F>
    fluent<Collection, T> fluent<Collection, T>::filter(F&& predicate) const {
        fluent<Collection, T> result;
        std::copy_if(
                collection_type::begin(),
                collection_type::end(),
                std::inserter(result, result.end()),
                predicate);
        return result;
    }

    template<template<class> class Collection, class T> template <class F>
    T fluent<Collection, T>::reduce(F&& binaryOp) const {
        if (collection_type::begin() == collection_type::end())
            return T();
        return std::accumulate(
                ++collection_type::begin(),
                collection_type::end(),
                *collection_type::begin(),
                binaryOp);
    }

    template<template<class> class Collection, class T> template <class F>
    T fluent<Collection, T>::reduce(F&& binaryOp, T const& first) const {
        return std::accumulate(
                collection_type::begin(),
                collection_type::end(),
                first,
                binaryOp);
    }

}
