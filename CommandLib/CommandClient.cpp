#include <cstdio>
#include <iostream>
#include "CommandClient.h"

using namespace CommandLib;
using asio::ip::tcp;

CommandClient::CommandClient(tcp::socket&& socket) : socket_(std::move(socket))
{
}

void CommandClient::send_request(std::string&& command) {
	CommandMessage commandMessage{ command, "str2" };
	commandMessage.send(socket_);
}

CommandMessage CommandClient::read_response() {
	auto response = CommandMessage::receive(socket_);
    return response;
}
