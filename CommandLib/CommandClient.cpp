#include <cstdio>
#include <iostream>
#include <vector>
#include "CommandLib.h"

using asio::ip::tcp;

CommandClient::CommandClient(tcp::socket&& socket) : socket_(std::move(socket))
{
}

void CommandClient::send_request(std::string&& command)
{
    command += '\0';
    asio::write(socket_, asio::buffer(command));
    std::cout << "Command sent: " << command << std::endl;
}

CommandResponse CommandClient::read_response()
{
    asio::streambuf response;
    asio::read_until(socket_, response, "\0");

    std::string serialized_response;
    std::istream response_stream(&response);
    std::getline(response_stream, serialized_response, '\0');

    return CommandResponse::deserialize(serialized_response);
}
