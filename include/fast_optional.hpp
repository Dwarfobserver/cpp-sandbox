
#pragma once


#include <type_traits>

namespace sc {

    namespace optional_traits {
        template <class Opt, class T, class Expr = void>
        constexpr bool is_optional = false;

        template <class Opt, class T>
        constexpr bool is_optional<Opt, T, std::enable_if_t<
                std::is_same_v<Opt, decltype(Opt())> &&
                std::is_same_v<T&, decltype(*Opt())> &&
                std::is_same_v<T*, decltype(Opt().operator->())> &&
                std::is_same_v<bool, decltype((bool) Opt())> &&
                !std::is_same_v<int, std::void_t< // Test expressions
                        decltype(Opt(std::declval<T>())),
                        decltype(std::declval<Opt>() = Opt())>>
        >> = true;
    }

    struct optional_emplace_tag {};

    template <auto EMPTY_VALUE>
    class fast_optional {
    public:
        using value_type = decltype(EMPTY_VALUE);

        constexpr fast_optional() noexcept : val{EMPTY_VALUE} {}
        constexpr fast_optional(optional_emplace_tag) noexcept : val{} {}

        template <class Arg, class...Args>
        constexpr fast_optional(Arg&& arg, Args&&...args) noexcept :
                val{std::forward<Arg>(arg), std::forward<Args>(args)...} {}

        constexpr fast_optional& operator=(fast_optional opt) noexcept { val = opt.val; return *this; }
        constexpr fast_optional& operator=(value_type val) noexcept { this->val = val; return *this; }

        value_type& operator*() noexcept { return val; }
        constexpr value_type operator*() const noexcept { return val; }
        value_type* operator->() noexcept { return &val; }
        value_type const* operator->() const noexcept { return &val; }
        constexpr operator bool() const noexcept { return val != EMPTY_VALUE; }
    private:
        value_type val;
    };
}
