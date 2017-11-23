
#pragma once

#include <udp/traits.hpp>
#include <queue>
#include <mutex>
#include <optional>


class PacketChannel {
public:
    struct Packet {
        std::vector<std::byte> data;
        uint32_t ip;
        uint16_t port;
    };
    void push(Packet&& p) {
        std::lock_guard<std::mutex> lock(packetsMutex);

        packetsQueue.push(std::move(p));
    }
    bool try_pop(Packet& p) {
        std::lock_guard<std::mutex> lock(packetsMutex);

        if (packetsQueue.empty())
            return false;

        p = packetsQueue.front();
        packetsQueue.pop();
        return true;
    }
private:
    std::queue<Packet> packetsQueue;
    std::mutex packetsMutex;
};


class MySocket {
public:
    explicit MySocket(PacketChannel& channel) : channel(channel) {}

    void send(void const* data, int size, uint32_t ip, uint16_t port) {
        PacketChannel::Packet p {};
        p.data.assign(data, data + size);
        p.ip = ip;
        p.port = port;
        channel.push(std::move(p));
    }
    void receive(int maxSize, void*& data, int& size, uint32_t& ip, uint16_t& port) {
        PacketChannel::Packet p {};
        if (!channel.try_pop(p)) {
            data = nullptr;
            return;
        }
        if (p.data.size() > maxSize) {
            throw std::runtime_error{"Received packet with size superior to maximal size"};
        }
        std::copy(p.data.begin(), p.data.end(), data);
        size = static_cast<int>(p.data.size());
        ip = p.ip;
        port = p.port;
    }
private:
    PacketChannel& channel;
};

namespace sc::udp::socket_traits {

    template <>
    void send(MySocket& socket, void const* data, int size, uint32_t ip, uint16_t port) {
        socket.send(data, size, ip, port);
    }

    template <>
    void receive(MySocket& socket, int maxSize, void*& data, int& size, uint32_t& ip, uint16_t& port) {
        socket.receive(maxSize, data, size, ip, port);
    }
}
