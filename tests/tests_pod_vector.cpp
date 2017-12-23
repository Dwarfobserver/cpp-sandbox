
#include "catch.hpp"
#include "pod_vector.hpp"


TEST_CASE("pod_vector correctness", "[pod_vector]") {
    sc::pod_vector<char> vec(10);

    REQUIRE(vec.size() == 10);

    vec.emplace_back();
    REQUIRE(vec.capacity() == 20);

    auto vec2 = std::move(vec);
    REQUIRE(vec2.capacity() == 20);

    auto vec3 = vec2;
    REQUIRE(vec3.capacity() == 11);
}

TEST_CASE("pod_vector performances", "[pod_vector]") {

}
