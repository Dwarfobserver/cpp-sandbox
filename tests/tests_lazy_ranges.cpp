
#include "catch.hpp"

#include <lazy_ranges.hpp>


TEST_CASE("lazy_ranges basics", "[lazy_ranges]") {
    std::vector<int> vec;
    for (int i = 0; i < 5; ++i) {
        vec.push_back(i);
    }

    REQUIRE(!sc::lazy_range(vec).find(6));

    auto sum = std::accumulate(vec.begin(), vec.end(), 0);
    auto sumCopy = sc::lazy_range(vec).reduce([] (int v1, int v2) { return v1 + v2; });
    REQUIRE(sum == sumCopy);

    std::vector<std::string> vecStr;

    sc::lazy_range(vec.begin(), vec.end())
        .filter([] (int val) -> bool { return val >= 2; })
        .map([] (int val) -> std::string { return std::to_string(val); })
        .copy(std::back_inserter(vecStr));

    REQUIRE(std::to_string(vec[2]) == vecStr[0]);
}

namespace {
    int movesCounter;
    int copiesCounter;

    class Dummy {
    public:
        Dummy() = default;

        Dummy(Dummy && old) noexcept {
            movesCounter++;
        }
        Dummy& operator=(Dummy && old) noexcept {
            movesCounter++;
            return *this;
        }
        Dummy(Dummy const& clone) {
            copiesCounter++;
        }
        Dummy& operator=(Dummy const& clone) {
            copiesCounter++;
            return *this;
        }
    };
}

TEST_CASE("lazy_ranges moves", "[lazy_ranges]") {
    std::vector<Dummy> vec(2);

    std::vector<Dummy> vecCopy;

    sc::lazy_range(vec)
        .map([] (Dummy const& dummy) { return dummy; })
        .map([] (Dummy && dummy) { return std::move(dummy); })
        .map([] (Dummy && dummy) { return std::move(dummy); })
        .copy(std::back_inserter(vecCopy));

    // 2 at first map, then 2 for copy
    REQUIRE(copiesCounter == 4);
}
