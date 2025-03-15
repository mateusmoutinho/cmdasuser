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

CommandResponse CommandClient::read_response() {
    asio::streambuf response;
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

    return CommandResponse::deserialize(serialized_response);
}

//CommandResponse CommandClient::read_response()
//{
//    asio::streambuf response;
//    asio::read_until(socket_, response, "\0");
//
//    std::string serialized_response;
//    std::istream response_stream(&response);
//    std::getline(response_stream, serialized_response, '\0');
//
//    return CommandResponse::deserialize(serialized_response);
//}

//CommandResponse CommandClient::read_response() {
//
//    const std::string marker = "MartWasntHere";
//
//    asio::streambuf response;
//    std::string serialized_response;
//    std::istream response_stream(&response);
//
//    while (true) {
//        asio::read_until(socket_, response, "\0");
//
//        std::string chunk;
//        std::getline(response_stream, chunk, '\0');
//        serialized_response += chunk;
//
//        if (serialized_response.find(marker) != std::string::npos) {
//            break;
//        }
//    }
//
//    // Remove the marker and everything after it from the response
//    size_t marker_pos = serialized_response.find(marker);
//    if (marker_pos != std::string::npos) {
//        serialized_response.resize(marker_pos);
//    }
//
//    return CommandResponse::deserialize(serialized_response);
//}