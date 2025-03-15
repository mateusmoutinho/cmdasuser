#pragma once

#include <asio.hpp>
#include "CommandMessage.h"
#include "CommandResponse.h"

namespace CommandLib {
    class CommandClient {
        asio::ip::tcp::socket socket_;
    public:
        CommandClient(asio::ip::tcp::socket&& socket);

        CommandMessage read_response();
        void send_request(std::string&& command);
    };
}

