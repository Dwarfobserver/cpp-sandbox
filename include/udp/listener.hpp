
#pragma once

#include "packet.hpp"
#include "traits.hpp"
#include <utils.hpp>
#include <memory>
#include <thread>
#include <functional>
#include <atomic>
#include <cstdint>
#include <stdint-gcc.h>


namespace sc::udp {

    template <class Socket, template <class> class PacketAllocator = std::allocator>
    class listener {
    public:
        template <class PacketHandler> // void operator(std::unique_ptr<packet>&& p, uint32_t ip, uint16_t port)
        void run(Socket& socket, PacketHandler&& handler, int packetMaxSize);
        void stop() noexcept;
        bool isFinished() const noexcept;
    private:
        std::atomic_bool finished;
        std::atomic_bool mustStop;
    };

    template <class Socket, template <class> class PacketAllocator> template<class PacketHandler>
    void listener<Socket, PacketAllocator>::run(Socket &socket, PacketHandler &&handler, int packetMaxSize) {
        finished.store(false);
        mustStop.store(false);

        // Finished only after destroyed ptrPacket data
        sc::defer hasFinished {[this] () noexcept { finished.store(true); }};

        auto ptrPacket = std::make_unique<packet>(packetMaxSize);
        while (!mustStop.load()) {
            if (!ptrPacket) {
                ptrPacket = std::make_unique<packet>(packetMaxSize);
            }
            void* data = ptrPacket->data();
            uint32_t ip;
            uint16_t port;
            socket_traits::receive<Socket>(socket, packetMaxSize, data, ptrPacket->end, ip, port);
            if (data) {
                handler(std::move(ptrPacket), ip, port);
            }
            else {
                using namespace std::chrono_literals;
                std::this_thread::sleep_for(3ms);
            }
        }
    }

    template <class Socket, template <class> class PacketAllocator>
    void listener<Socket, PacketAllocator>::stop() {
        mustStop.store(true);
    }

    template <class Socket, template <class> class PacketAllocator>
    bool listener<Socket, PacketAllocator>::isFinished() const {
        return finished.load();
    }

}
