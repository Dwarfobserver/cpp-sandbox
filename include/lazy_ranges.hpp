
#pragma once

#include <algorithm>
#include <numeric>
#include <optional>


namespace sc {

    template <class It> class lazy_range;

    template <class T, class FNext, class Range>
    class rec_lazy_range {
        friend Range;
    public:
        template <class OutputIt>
        void copy(OutputIt&& it);

        template <class UnaryOp>
        auto map(UnaryOp&& f);

        template <class Predicate>
        auto filter(Predicate&& f);

        std::optional<T> find(T const& val);

        template <class Predicate, class =
            std::enable_if_t<!std::is_convertible_v<Predicate, T>>>
        std::optional<T> find(Predicate&& f);

        template <class BinaryOp>
        std::optional<T> reduce(BinaryOp&& f);

        template <class BinaryOp>
        T reduce(BinaryOp&& f, T const& val);
    private:
        FNext next;

        explicit rec_lazy_range(FNext&& f);
    };

    template <class It>
    class lazy_range {
        using T = typename It::value_type;
        using opt_t = std::optional<T>;
    public:
        template <class Collection>
        explicit lazy_range(Collection const& collection) :
                 lazy_range(collection.cbegin(), collection.cend()) {}

        lazy_range(It begin, It end) :
                begin(begin), end(end) {}

        template <class OutputIt>
        void copy(OutputIt&& it);

        template <class UnaryOp>
        auto map(UnaryOp&& f);

        template <class Predicate>
        auto filter(Predicate&& f);

        std::optional<T> find(T const& val);

        template <class Predicate, class =
            std::enable_if_t<!std::is_convertible_v<Predicate, T>>>
        std::optional<T> find(Predicate&& f);

        template <class BinaryOp>
        std::optional<T> reduce(BinaryOp&& f);

        template <class BinaryOp>
        T reduce(BinaryOp&& f, T const& val);
    private:
        It begin;
        It end;
    };

    template<class Collection> lazy_range(Collection) -> lazy_range<decltype(std::declval<Collection>().cbegin())>;

    // ______________
    // Implementation

    // Recursive range

    template <class T, class FNext, class Range> template <class OutputIt>
    void rec_lazy_range<T, FNext, Range>::copy(OutputIt&& it) {
        auto val = next();
        while (val) {
            it = *val;
            ++it;
            val = next();
        }
    }

    template <class T, class FNext, class Range> template<class UnaryOp>
    auto rec_lazy_range<T, FNext, Range>::map(UnaryOp &&f) {
        using opt_t = std::optional<decltype(f(std::declval<T>()))>;
        auto nextFunc = [this, f = std::forward<UnaryOp>(f)] () -> opt_t {
            auto val = next();
            return (val)
                   ? opt_t(f(std::move(*val)))
                   : opt_t();
        };
        return rec_lazy_range<T, decltype(nextFunc), rec_lazy_range<T, FNext, Range>>(std::move(nextFunc));
    }

    template <class T, class FNext, class Range> template<class Predicate>
    auto rec_lazy_range<T, FNext, Range>::filter(Predicate &&f) {
        using opt_t = std::optional<T>;

        auto nextFunc = [this, f = std::forward<Predicate>(f)] () -> opt_t {
            auto val = next();
            if (!val) return opt_t();
            while (!f(*val)) {
                val = next();
                if (!val) return opt_t();
            }
            return opt_t(std::move(val));
        };
        return rec_lazy_range<T, decltype(nextFunc), rec_lazy_range<T, FNext, Range>>(std::move(nextFunc));
    }

    template <class T, class FNext, class Range> template<class Predicate, class SFINAE>
    std::optional<T> rec_lazy_range<T, FNext, Range>::find(Predicate &&f) {
        auto val = next();
        if (!val) return {};
        while (!f(*val)) {
            val = next();
            if (!val) return {};
        }
        return val;
    }

    template <class T, class FNext, class Range>
    std::optional<T> rec_lazy_range<T, FNext, Range>::find(const T &val) {
        auto opt = next();
        if (!opt) return {};
        while (*opt != val) {
            opt = next();
            if (!opt) return {};
        }
        return opt;
    }

    template <class T, class FNext, class Range> template<class BinaryOp>
    std::optional<T> rec_lazy_range<T, FNext, Range>::reduce(BinaryOp &&f) {
        auto opt = next();
        if (!opt) return {};

        auto v = *next();
        opt = next();
        while (opt) {
            v = f(v, *opt);
            opt = next();
        }
        return v;
    }

    template <class T, class FNext, class Range> template<class BinaryOp>
    T rec_lazy_range<T, FNext, Range>::reduce(BinaryOp &&f, const T &val) {
        auto v = *next();
        auto opt = next();
        while (opt) {
            v = f(v, *opt);
            opt = next();
        }
        return v;
    }

    template <class T, class FNext, class Range>
    rec_lazy_range<T, FNext, Range>::rec_lazy_range(FNext&& f) :
            next(std::forward<FNext>(f))
    {}

    // Original range

    template <class It> template<class OutputIt>
    void lazy_range<It>::copy(OutputIt&& it) {
        std::copy(begin, end, it);
    }

    template <class It> template<class UnaryOp>
    auto lazy_range<It>::map(UnaryOp &&f) {
        using map_t = decltype(f(std::declval<T>()));
        using opt_map_t = std::optional<map_t>;

        auto nextFunc = [this, f = std::forward<UnaryOp>(f)] () -> opt_map_t {
            if (begin == end) return opt_map_t();
            return opt_map_t(f(*(begin++)));
        };
        return rec_lazy_range<map_t, decltype(nextFunc), lazy_range<It>>(std::move(nextFunc));
    }

    template <class It> template<class Predicate>
    auto lazy_range<It>::filter(Predicate &&f) {
        auto nextFunc = [this, f = std::forward<Predicate>(f)] () -> opt_t {
            if (begin == end) return opt_t();
            auto val = *(begin++);
            while (!f(val)) {
                if (begin == end) return opt_t();
                val = *(begin++);
            }
            return opt_t(std::move(val));
        };
        return rec_lazy_range<T, decltype(nextFunc), lazy_range<It>>(std::move(nextFunc));
    }

    template <class It>
    typename lazy_range<It>::opt_t lazy_range<It>::find(T const& val) {
        auto it = std::find(begin, end, val);
        return it != end ? opt_t(*it) : opt_t();
    }

    template <class It> template<class Predicate, class SFINAE>
    typename lazy_range<It>::opt_t lazy_range<It>::find(Predicate &&f) {
        auto it = std::find_if(begin, end, std::forward<Predicate>(f));
        return it != end ? opt_t(*it) : opt_t();
    }

    template <class It> template<class BinaryOp>
    typename lazy_range<It>::opt_t lazy_range<It>::reduce(BinaryOp &&f) {
        if (begin == end) return opt_t();
        return std::accumulate(begin + 1, end, *begin, std::forward<BinaryOp>(f));
    }

    template <class It> template<class BinaryOp>
    typename lazy_range<It>::T lazy_range<It>::reduce(BinaryOp &&f, T const& val) {
        return std::accumulate(begin, end, val, std::forward<BinaryOp>(f));
    }

}
