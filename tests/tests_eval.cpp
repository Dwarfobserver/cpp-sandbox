
#include "catch.hpp"
#include <eval.hpp>
#include <future>


TEST_CASE("eval simplest program", "[eval]") {
    int code = 42;
    try {
        code = sc::eval("int main() { return -3; }");
    }
    catch(std::exception const& e) {
        FAIL(e.what());
    }
    REQUIRE(code == -3);
}

TEST_CASE("eval multiple programs", "[eval]") {
    std::vector<std::string> sources;
    for (int i = 1; i <= 3; ++i) {
        sources.push_back("int main() { return " + std::to_string(i) + "; }");
    }
    std::vector<std::future<int>> results;
    for (auto const& src : sources) {
        results.push_back(std::async(std::launch::async, [&] {
            return sc::eval(src);
        }));
    }
    int sum = 0;
    try {
        for (auto& fut : results) sum += fut.get();
    }
    catch(std::exception const& e) {
        FAIL(e.what());
    }
    REQUIRE(sum == 6);
}
