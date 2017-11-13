
#pragma once


#include <type_traits>

namespace sc {

    namespace optional_traits {
        template <class Opt, class T, class Expr = void>
        constexpr bool is_optional = false;

        template <class Opt, class T>
        constexpr bool is_optional<Opt, T, std::enable_if_t<
                std::is_same_v<Opt, decltype(Opt())> &&
                std::is_same_v<T, decltype(*Opt())> &&
                std::is_same_v<T*, decltype(Opt().operator->())> &&
                std::is_same_v<bool, decltype((bool) Opt())> &&
                !std::is_same_v<int, std::void_t< // Test expressions
                        decltype(Opt(std::declval<T>())),
                        decltype(std::declval<Opt>() = Opt())>>
        >> = true;
    }

    template <class T, T EMPTY_VALUE>
    class optional {
    public:
        constexpr optional() noexcept : val{EMPTY_VALUE} {}
        constexpr optional(optional const& opt) noexcept : val{opt.val} {}
        constexpr optional(T val) noexcept : val{val} {}
        constexpr optional& operator=(optional opt) noexcept { val = opt.val; return *this; }
        constexpr optional& operator=(T val) noexcept { this->val = val; return *this; }

        constexpr T operator*() const noexcept { return val; }
        T* operator->() noexcept { return &val; }
        T const* operator->() const noexcept { return &val; }
        constexpr operator bool() const noexcept { return val != EMPTY_VALUE; }
    private:
        T val;
    };
}
