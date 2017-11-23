
#pragma once

#include <cstdint>


namespace sc::udp {

    namespace socket_traits {

        template <class Socket>
        static void send(Socket& socket, void const* data, int size, uint32_t ip, uint16_t port);

        template <class Socket>
        static void receive(Socket& socket, int maxSize, void*& data, int& size, uint32_t& ip, uint16_t& port);
    }

}
