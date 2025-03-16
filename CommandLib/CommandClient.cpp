#include <cstdio>
#include <iostream>
#include "CommandClient.h"

using asio::ip::tcp;
using namespace CommandLib;

const std::string CommandClient::Ctrl_C = "[Ctrl-C]";
CommandClient* CommandClient::pSignal_handler = nullptr;

CommandClient::CommandClient(tcp::socket&& socket) : socket_(std::move(socket))
{
}

void CommandClient::send_request(const std::string& command) {
	CommandMessage commandMessage{ command, "str2" };
	commandMessage.send(socket_);
}

CommandMessage CommandClient::read_response() {
	auto response = CommandMessage::receive(socket_);
    return response;
}

void CommandClient::on_signal_received(int signal) {

	if (signal == SIGINT) {
		if (CommandClient::pSignal_handler != nullptr) {
			int n = 3;
			// CommandClient::pSignal_handler->send_request(CommandClient::Ctrl_C);
		}
	}
}
