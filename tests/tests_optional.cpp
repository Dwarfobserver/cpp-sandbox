
#include "catch.hpp"

#include <optional.hpp>


TEST_CASE("optional constexpr", "[optional]") {
    using data_t = int;
    using opt_t = sc::optional<data_t(-1)>;

    static_assert(sc::optional_traits::is_optional<opt_t, data_t>);

    constexpr auto getValidValue{[] () -> data_t { return 2; }};
    auto getInvalidValue{[] () -> data_t { return -1; }};

    constexpr opt_t opt1{getValidValue()};
    static_assert(opt1);
    opt_t opt2 = getInvalidValue();
    REQUIRE(!opt2);
    REQUIRE(!opt_t());

    using opt_ptr_t = sc::optional<(int*)(nullptr)>;
    int i = 2;
    opt_ptr_t p = &i;
    REQUIRE((bool) p);
    REQUIRE(!opt_ptr_t(nullptr));
}
