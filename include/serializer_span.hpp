
#pragma once

#include <utils.hpp>
#include <cstddef>
#include <type_traits>
#include <utility>
#include <algorithm>
#include <cassert>


namespace sc::serializer {

    enum class policy { // TODO Allow user policy ?
        binary,
        string
    };

    namespace detail {
        enum io_enum_t {
            in =  1 << 0,
            out = 1 << 1
        };

        // Tag inheritance to detect a category

        template <policy Policy, int IO>
        struct span_category;
        template <policy Policy>
        struct span_category<Policy, in> {};
        template <policy Policy>
        struct span_category<Policy, out> {};
        template <policy Policy>
        struct span_category<Policy, in|out> :
                span_category<Policy, in>, span_category<Policy, out> {};

        template <class Span, policy Policy, int IO>
        constexpr bool is_from_category = std::is_base_of_v<detail::span_category<Policy, IO>, Span>;

        // Base span class

        template <policy Policy, int IO>
        struct span : span_category<Policy, IO> {
            span() : data{nullptr}, size{0} {}
            span(std::byte* data, int size) : data{data}, size{size} {}
            std::byte* data;
            int size;
        };
    }

    using binary_ispan = detail::span<policy::binary, detail::in>;
    using binary_ospan = detail::span<policy::binary, detail::out>;
    using binary_iospan = detail::span<policy::binary, detail::in | detail::out>;

    // Generic io operations

    template <class Span, class T>
    auto operator<<(Span& span, T const& data)
    -> return_if_t<Span,
            detail::is_from_category<Span, policy::binary, detail::out>
    >&
    {
        assert(span.size >= sizeof(T) &&
               "The span have a too little size to serialize the data of type T.");

        new (span.data) T(data);
        span.data += sizeof(T);
        span.size -= sizeof(T);
        return span;
    };

    template <class Span, class T>
    auto operator>>(Span& span, T& data)
    -> return_if_t<Span,
            detail::is_from_category<Span, policy::binary, detail::in>
    >&
    {
        assert(span.size >= sizeof(T) &&
               "The span have a too little size to serialize the data of type T.");

        data = *reinterpret_cast<T*>(span.data);
        span.data += sizeof(T);
        span.size -= sizeof(T);
        return span;
    };

    // std::vector

    template <class Span, class T>
    auto operator<<(Span& span, std::vector<T> const& data)
    -> return_if_t<Span,
            detail::is_from_category<Span, policy::binary, detail::out>
    >&
    {
        assert(span.size >= sizeof(int) + sizeof(T) * data.size() &&
               "The span have a too little size to serialize the data of type vector<T>.");

        span << static_cast<uint32_t>(data.size());
        for (auto const& val : data) {
            span << val;
        }
        return span;
    };

    template <class Span, class T>
    auto operator>>(Span& span, std::vector<T>& data)
    -> return_if_t<Span,
            detail::is_from_category<Span, policy::binary, detail::in>
    >&
    {
        assert(span.size >= sizeof(int) + sizeof(T) * data.size() &&
               "The span have a too little size to serialize the data of type vector<T>.");

        uint32_t size;
        span >> size;
        data.resize(size);
        for (auto& val : data) {
            span >> val;
        }
        return span;
    };

}
