
#include "catch.hpp"
#include <compact_map.hpp>
#include <make_string.hpp>
#include <iostream>


using namespace std::literals;

TEST_CASE("compact_map basics", "[compact_map]") {
    sc::compact_map<int, std::string> map;

    map.reserve(6);

    REQUIRE(map.insert({2, "de"s}).second);
    REQUIRE(map.insert({1, "sire"s}).second);
    REQUIRE(map.insert({3, "nez"s}).first->second == "nez");
    REQUIRE(!map.insert({1, "existing key"s}).second);

    auto it = map.begin();
    REQUIRE(it->second == "sire");
    REQUIRE((++it)->second == "de");
    REQUIRE((++it)->second == "nez");
    REQUIRE(++it == map.end());

    REQUIRE(map[4].empty());

    REQUIRE(map.erase(map.find(2))->first == 3);
}

