
#include "catch.hpp"
#include <eval.hpp>


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
