#pragma once

#include <string>

namespace CommandLib {
    class CommandMessage {
        std::string str2_;
        std::string payload_;

    public:
        CommandMessage(const std::string& payload, const std::string& str2);

        std::string serialize() const;
        void send(asio::ip::tcp::socket& socket) const;

        const std::string& get_payload() const { return payload_; }
        const std::string& getStr2() const { return str2_; }

        static CommandMessage deserialize(const std::string& data);
        static CommandMessage receive(asio::ip::tcp::socket& socket);
        static CommandMessage try_receive(asio::ip::tcp::socket& socket);
    };
}