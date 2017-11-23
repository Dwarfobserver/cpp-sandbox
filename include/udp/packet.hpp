
#pragma once

#include <cstddef>
#include <vector>


namespace sc::udp {

    template <template <class> class Allocator = std::allocator>
    struct packet {
        std::vector<std::byte, Allocator<std::byte>> vector;
        int begin;
        int end;

        explicit packet(int size) : vector(size), begin(0), end(size) {}
    };

}
