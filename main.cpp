
#include <iostream>
#include <functional>


int main() {
    [out = std::ref(std::cout << "Hello ")] {
        out << "World !" << std::endl;
    }();
}
