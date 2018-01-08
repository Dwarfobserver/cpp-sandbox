
#pragma once

#include <type_traits>
#include <cstdint>


namespace flag_operators {

    namespace detail {

        namespace impl {

            template<class Enum, class Expr = void>
            struct integer_t;

            template<class Enum>
            struct integer_t<Enum, std::enable_if_t<
                    sizeof(Enum) == 1
            >> {
                using type = uint8_t;
            };

            template<class Enum>
            struct integer_t<Enum, std::enable_if_t<
                    sizeof(Enum) == 2
            >> {
                using type = uint16_t;
            };

            template<class Enum>
            struct integer_t<Enum, std::enable_if_t<
                    sizeof(Enum) == 3 || sizeof(Enum) == 4
            >> {
                using type = uint32_t;
            };

            template<class Enum>
            struct integer_t<Enum, std::enable_if_t<
                    sizeof(Enum) >= 5 && sizeof(Enum) <= 8
            >> {
                using type = uint64_t;
            };

        }

        template<class Enum>
        using integer_t = typename impl::integer_t<Enum>::type;

        template<class Enum>
        auto int_of(Enum e) {
            return static_cast<detail::integer_t<Enum>>(e);
        }
        template<class Enum>
        auto& int_ref_of(Enum& e) {
            return *static_cast<detail::integer_t<Enum>*>(&e);
        }
    }

    template<class Enum>
    Enum operator|(Enum lhs, Enum rhs) {
        return static_cast<Enum>(detail::int_of(lhs) | detail::int_of(rhs));
    }

    template<class Enum>
    Enum operator&(Enum lhs, Enum rhs) {
        return static_cast<Enum>(detail::int_of(lhs) & detail::int_of(rhs));
    }

    template<class Enum>
    Enum operator^(Enum lhs, Enum rhs) {
        return static_cast<Enum>(detail::int_of(lhs) ^ detail::int_of(rhs));
    }

    template<class Enum>
    Enum operator~(Enum e) {
        return static_cast<Enum>(~detail::int_of(e));
    }

    template<class Enum>
    Enum& operator|=(Enum& lhs, Enum rhs) {
        return static_cast<Enum&>(detail::int_ref_of(lhs) |= detail::int_of(rhs));
    }

    template<class Enum>
    Enum& operator&=(Enum& lhs, Enum rhs) {
        return static_cast<Enum&>(detail::int_ref_of(lhs) &= detail::int_of(rhs));
    }

    template<class Enum>
    Enum& operator^=(Enum& lhs, Enum rhs) {
        return static_cast<Enum&>(detail::int_ref_of(lhs) ^= detail::int_of(rhs));
    }

}
