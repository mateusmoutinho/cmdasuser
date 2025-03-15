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

    /*command += '\0';
    asio::write(socket_, asio::buffer(command));
    std::cout << "Command sent: " << command << std::endl;*/
}

CommandMessage CommandClient::read_response() {
	auto response = CommandMessage::receive(socket_);
    return response;

    /*asio::streambuf response;
    std::string serialized_response;
    std::istream response_stream(&response);

    while (true) {
        asio::error_code error;
        asio::read_until(socket_, response, "\0", error);

        if (error && error != asio::error::not_found) {
            throw asio::system_error(error);
        }

        std::string chunk;
        std::getline(response_stream, chunk, '\0');
        serialized_response += chunk;

        if (error != asio::error::not_found) {
            break;
        }
    }

    return CommandResponse::deserialize(serialized_response);*/
}
