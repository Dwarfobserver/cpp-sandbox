
#pragma once

#include <utils.hpp>
#include <cstddef>
#include <type_traits>
#include <utility>
#include <algorithm>
#include <cassert>
#include <iostream>


namespace sc::serializer {

    // A span capacity is determined by it's storage, policy and category (io flags)

    enum class storage {
        invalid = 0,
        binary,
    };
    enum class policy {
        invalid = 0,
        throwing,
        unsafe,
        safe,
    };
    struct category {
        // Flag enum
        enum type : int {
            in  = 1 << 0,
            out = 1 << 1
        };
    };

    class overflow_exception : std::runtime_error {
    public:
        explicit overflow_exception(int dataSize, int spanSize, int category_op) :
                std::runtime_error{
                        "Tried to serialize data of size " + std::to_string(dataSize) +
                        (category_op == category::in ? " in" : " from") +
                        " a span serializer of size " + std::to_string(spanSize)
        }, spanSize{spanSize}, dataSize{dataSize} {}
        int spanSize;
        int dataSize;
    };

    namespace detail {
        // Tag inheritance to detect a category

        template<storage Storage>
        struct span_storage {};

        template<policy Policy>
        struct span_policy {};

        template<>
        struct span_policy<policy::safe> {};

        template<int IO>
        struct span_category {};
        template<>
        struct span_category<category::in | category::out> :
                span_category<category::in>, span_category<category::out> {};

        // Detect if has a trait

        template<class Span, policy Policy>
        constexpr bool has_policy =
                std::is_base_of_v<span_policy<Policy>, Span>;

        template<class Span, storage Storage>
        constexpr bool has_storage =
                std::is_base_of_v<span_storage<Storage>, Span>;

        template<class Span, int IO>
        constexpr bool has_category =
                std::is_base_of_v<span_category<IO>, Span>;

        // Detect spans

        template <class Span, class SFINAE = void>
        constexpr bool is_span = false;

        template <class Span>
        constexpr bool is_span<Span, std::enable_if_t<
                has_category<Span, category::in> ||
                has_category<Span, category::out>
        >> = true;

        // Base span class

        template <storage Storage, policy Policy, int Category>
        struct span : span_policy<Policy>, span_storage<Storage>, span_category<Category> {
            static constexpr storage storage_type = Storage;
            static constexpr policy policy_type = Policy;
            static constexpr int category_type = Category;

            span() : span_policy<Policy>(),
                     begin{nullptr},
                     end{nullptr} {}

            span(void* begin, void const* end) :
                    span_policy<Policy>(),
                    begin{reinterpret_cast<std::byte*>(begin)},
                    end{reinterpret_cast<std::byte const*>(end)} {}

            std::byte *begin;
            std::byte const *end;

            int size() const noexcept { return static_cast<int>(end - begin); }

            // Any span can be constructed from / assigned to another span

            template<class OtherSpan>
            explicit span(OtherSpan const &rhs) : begin{rhs.begin}, end{rhs.end} {};

            template<class OtherSpan>
            return_if_t<is_span<OtherSpan>,
            span>& operator=(OtherSpan const &rhs) {
                begin = rhs.begin;
                end = rhs.end;
                return *this;
            }

            // Casts to another spans

            template <storage NewStorage>
            auto cast_to() { return span<NewStorage, Policy, Category>(this); }
            template <policy NewPolicy>
            auto cast_to() { return span<Storage, NewPolicy, Category>(this); }
            template <int NewCategory>
            auto cast_to() { return span<Storage, Policy, NewCategory>(this); }
            template <storage NewStorage, policy NewPolicy>
            auto cast_to() { return span<NewStorage, NewPolicy, Category>(this); }
        };
    }

    // Convenient typedefs

    template <storage Storage, policy Policy>
    using ispan = detail::span<Storage, Policy, category::in>;
    template <storage Storage, policy Policy>
    using ospan = detail::span<Storage, Policy, category::out>;
    template <storage Storage, policy Policy>
    using iospan = detail::span<Storage, Policy, category::in | category::out>;

    template <policy Policy>
    using binary_ispan =  ispan<storage::binary, Policy>;
    template <policy Policy>
    using binary_ospan =  ospan<storage::binary, Policy>;
    template <policy Policy>
    using binary_iospan = iospan<storage::binary, Policy>;

    namespace detail {

        // Serializable traits class

        template <storage Storage, class T>
        struct default_traits;

        // TODO Make a macro to generate default types

        template <class T>
        struct default_binary_traits {
            template <class Span>
            static void out(Span& span, T const& val);
            template <class Span>
            static void in(Span& span, T& val);
        };

        template <class T> template<class Span>
        void default_binary_traits<T>::out(Span &span, const T &val) {
            if constexpr (Span::policy_type == policy::safe) {
                if (span.end - span.begin < (int) sizeof(T)) {
                    span.end = nullptr;
                    return;
                }
            }
            else if constexpr (Span::policy_type == policy::throwing) {
                if (span.end - span.begin < (int) sizeof(T)) {
                    throw overflow_exception(sizeof(T), span.end - span.begin, category::out);
                }
            }
            auto pVal = reinterpret_cast<std::byte const*>(&val);
            std::copy(pVal, pVal + sizeof(T), span.begin);
            span.begin += sizeof(T);
        }
        template <class T> template<class Span>
        void default_binary_traits<T>::in(Span &span, T &val) {
            if constexpr (Span::policy_type == policy::safe) {
                if (span.end - span.begin < (int) sizeof(T)) {
                    span.valid = nullptr;
                    return;
                }
            }
            else if constexpr (Span::policy_type == policy::throwing) {
                if (span.end - span.begin < (int) sizeof(T)) {
                    throw overflow_exception(sizeof(T), span.end - span.begin, category::in);
                }
            }
            auto pVal = const_cast<std::byte*>(reinterpret_cast<std::byte const*>(&val));
            std::copy(span.begin, span.begin + sizeof(T), pVal);
            span.begin += sizeof(T);
        }

        template <class T>
        struct default_traits<storage::binary, T> { using type = default_binary_traits<T>; };

    }

    // User definition

    template <storage Storage, class T>
    struct serializable_traits { using type = typename detail::default_traits<Storage, T>::type; };

    // IO operators

    namespace detail {
        template <class Span, class T, int Category>
        Span& make_io(Span& span, T& data) {
            using traits_t = typename serializable_traits<Span::storage_type, T>::type;
            if constexpr (Span::policy_type == policy::safe) {
                if (span.end == nullptr) return span;
            }
            if constexpr (Category == category::out)
                traits_t::out(span, data);
            else traits_t::in(span, data);
            return span;
        };
    }

    template <class Span, class T>
    auto operator<<(Span& span, T const& data)
    -> return_if_t<
            detail::has_category<Span, category::out>
    ,Span>& {
        return detail::make_io<Span, T, category::out>(span, const_cast<T&>(data));
    };

    template <class Span, class T>
    auto operator>>(Span& span, T& data)
    -> return_if_t<
            detail::has_category<Span, category::in>
    ,Span>& {
        return detail::make_io<Span, T, category::in>(span, data);
    };

    // std::vector

    namespace detail {
        template<class T>
        struct vector_binary_traits {
            template<class Span>
            static void out(Span &span, std::vector<T> const &val);

            template<class Span>
            static void in(Span &span, std::vector<T> &val);
        };

        template<class T>
        template<class Span>
        void vector_binary_traits<T>::out(Span &span, std::vector<T> const &vec) {
            span << static_cast<uint32_t>(vec.size());
            for (auto const &val : vec) {
                span << val;
            }
        }

        template<class T>
        template<class Span>
        void vector_binary_traits<T>::in(Span &span, std::vector<T> &vec) {
            uint32_t size;
            span >> size;
            vec.resize(size);
            for (auto const &val : vec) {
                span >> val;
            }
        }
    }

    template <class T>
    struct serializable_traits<storage::binary, std::vector<T>> {
        using type = detail::vector_binary_traits<T>;
    };

    // std::string

    namespace detail {
        struct string_binary_traits {
            template<class Span>
            static void out(Span &span, std::string const &str);

            template<class Span>
            static void in(Span &span, std::string &str);
        };

        template<class Span>
        void string_binary_traits::out(Span &span, std::string const &str) {
            span << static_cast<uint32_t>(str.size());
            std::copy(
                    str.begin(),
                    str.end(),
                    reinterpret_cast<char*>(span.begin));
            span.begin += str.size();
        }

        template<class Span>
        void string_binary_traits::in(Span &span, std::string &str) {
            uint32_t size;
            span >> size;
            str.assign(
                    reinterpret_cast<char const*>(span.begin),
                    reinterpret_cast<char const*>(span.begin + size));
            span.begin += str.size();
        }
    }

    template <>
    struct serializable_traits<storage::binary, std::string> {
        using type = detail::string_binary_traits;
    };

}
