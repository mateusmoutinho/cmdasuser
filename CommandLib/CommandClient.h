#pragma once

#include <asio.hpp>
#include "CommandResponse.h"

class CommandClient {
    asio::ip::tcp::socket socket_;
public:
    CommandClient(asio::ip::tcp::socket&& socket);

    CommandResponse read_response();
	void send_request(std::string&& command);
};

