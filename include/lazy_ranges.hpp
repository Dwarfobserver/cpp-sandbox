
#pragma once


#include <algorithm>

namespace sc {

    template <class It, class Range, class...Ranges> class rec_lazy_range;

    template <class It>
    class lazy_range {
        using T = typename It::value_type;
        friend class rec_lazy_range<It, lazy_range<It>>;
    public:
        lazy_range(It const& begin, It const& end) :
                begin(begin), end(end) {}
        lazy_range(It && begin, It const& end) :
                begin(std::move(begin)), end(end) {}
        lazy_range(It const& begin, It && end) :
                begin(begin), end(std::move(end)) {}
        lazy_range(It && begin, It && end) noexcept :
                begin(std::move(begin)), end(std::move(end)) {}

        template <class F>
        rec_lazy_range<It, lazy_range<It>> map(F&& f);
    private:
        It begin;
        It end;

        lazy_range(lazy_range<It>&& moved) noexcept :
                begin(std::move(moved.begin)), end(std::move(moved.end)) {}
        T const& next();
        bool finished() const noexcept;
    };

    template <class It, class Range, class...Ranges>
    class rec_lazy_range {
        friend Range;
    public:
        template <class OutputIt>
        void eval(OutputIt it);
    private:
        Range& last_range;

        explicit rec_lazy_range(Range& range) noexcept;
    };

    // ______________
    // Implementation

    template <class It> template<class F>
    rec_lazy_range<It, lazy_range<It>> lazy_range<It>::map(F &&f) {
        return rec_lazy_range<It, lazy_range<It>>(*this);
    }

    template <class It>
    typename lazy_range<It>::T const& lazy_range<It>::next() {
        const auto& val = *begin;
        begin++;
        return val;
    }

    template <class It>
    bool lazy_range<It>::finished() const noexcept {
        return begin == end;
    }

    // Recursive range

    template <class It, class Range, class...Ranges> template <class OutputIt>
    void rec_lazy_range<It, Range, Ranges...>::eval(OutputIt it) {
        while (!last_range.finished()) {
            it = last_range.next();
            ++it;
        }
    }

    template <class It, class Range, class...Ranges>
    rec_lazy_range<It, Range, Ranges...>::rec_lazy_range(Range& range) noexcept :
            last_range(range)
    {}

}
