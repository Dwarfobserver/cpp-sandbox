
#include "catch.hpp"

#include <udp/listener.hpp>
#include <udp/sender.hpp>
#include "udp_socket_impl.hpp"

#include <future>
#include <iostream>


TEST_CASE("udp_sockets basics", "udp_sockets") {
    using p_packet_t = std::unique_ptr<sc::udp::packet>;

    PacketChannel channel;

    MySocket listeningSocket(channel);
    sc::udp::listener listener;

    auto listenerFuture = std::async(std::launch::async, [&] {

        listener.run(listeningSocket, [] (p_packet_t&& p, uint32_t ip, uint16_t port) {
            std::cout << "packet received from " << ip << ":" << port << " of size " << p->end - p->begin << std::endl;
        }, 1024);
    });


    std::queue<std::vector<std::byte>> packetsQueue;
    std::mutex packetsMutex;

    MySocket sendingSocket(channel);
    sc::udp::sender sender;

    auto senderFuture = std::async(std::launch::async, [&] {

        sender.run(sendingSocket, [&] (p_packet_t& p, uint32_t& ip, uint16_t& port) {
            std::lock_guard<std::mutex> lock(packetsMutex);

            if (packetsQueue.empty()) {
                p.reset(nullptr);
            }
            else {
                ip = 127'0'0'1; // How to octo ?
                port = 12345;
                p = std::make_unique<sc::udp::packet>(static_cast<int>(packetsQueue.front().size()));
                std::copy(packetsQueue.front().begin(), packetsQueue.front().end(), p->vector.begin());
            }
        });
    });

    for (int i = 0; i < 10; ++i) {
        {
            std::lock_guard<std::mutex> lock(packetsMutex);
            packetsQueue.emplace(5 * i);
        }
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(5ms);
    }

    sender.stop();
    senderFuture.get();

    listener.stop();
    listenerFuture.get();
}

