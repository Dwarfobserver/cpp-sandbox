
#pragma once

#include "packet.hpp"
#include "traits.hpp"
#include <memory>
#include <thread>
#include <functional>
#include <atomic>
#include <utils.hpp>


namespace sc::udp {

    template <class Socket>
    class sender {
    public:
        template <class PacketPoller> // void operator(std::unique_ptr<packet>& p, uint32_t& ip, uint16_t& port)
        void run(Socket& socket, PacketPoller&& poller);
        void stop() noexcept;
        bool isFinished() const noexcept;
    private:
        std::atomic_bool finished;
        std::atomic_bool mustStop;
    };

    template <class Socket> template<class PacketPoller>
    void sender<Socket>::run(Socket &socket, PacketPoller&& poller) {
        finished.store(false);
        mustStop.store(false);

        sc::defer hasFinished {[] () noexcept { finished.store(true); }};

        while (!mustStop.load()) {
            std::unique_ptr<packet> ptrPacket {};
            uint32_t ip;
            uint16_t port;
            poller(ptrPacket, ip, port);
            if (ptrPacket) {
                socket_traits::send<Socket>(socket, ptrPacket->vector.data() + ptrPacket->begin, ptrPacket->end, ip, port);
            }
            else {
                using namespace std::chrono_literals;
                std::this_thread::sleep_for(3ms);
            }
        }
    }

    template <class Socket>
    void sender<Socket>::stop() {
        mustStop.store(true);
    }

    template <class Socket>
    bool sender<Socket>::isFinished() const {
        return finished.load();
    }

}
