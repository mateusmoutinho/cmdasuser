#include <cstdio>
#include <iostream>
#include "CommandClient.h"

using asio::ip::tcp;
using namespace CommandLib;

const std::string CommandClient::Ctrl_C = "[Ctrl-C]";
CommandClient* CommandClient::pSignal_handler_ = nullptr;

CommandClient::CommandClient(asio::io_context& io_context, tcp::socket&& socket) : 
	socket_(std::move(socket)), signals_(io_context, SIGINT) {
	signals_.async_wait([this](const asio::error_code& error, int signal_number) {
		handle_signal(error, signal_number);
		});
}

void CommandClient::send_request(const std::string& command) {
	CommandMessage commandMessage{ command, "str2" };
	commandMessage.send(socket_);
}

CommandMessage CommandClient::read_response() {
	auto response = CommandMessage::receive(socket_);
    return response;
}

void CommandClient::handle_signal(const asio::error_code& error, int signal_number) {
	if (!error && signal_number == SIGINT) {
		//send_request(CommandClient::Ctrl_C);

		signals_.async_wait([this](const asio::error_code& error, int signal_number) {
			handle_signal(error, signal_number);
		});
	}
}

void CommandClient::on_signal_received(int signal) {

	if (signal == SIGINT) {
		if (CommandClient::pSignal_handler_ != nullptr) {
			int n = 3;
			// CommandClient::pSignal_handler->send_request(CommandClient::Ctrl_C);
		}
	}
}
