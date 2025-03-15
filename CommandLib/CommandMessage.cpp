#include <iostream>
#include <asio.hpp>
#include "Exceptions.h"
#include "CommandMessage.h"

using asio::ip::tcp;
using namespace CommandLib;

CommandMessage::CommandMessage(const std::string& payload, const std::string& str2) : 
    payload_(payload), str2_(str2) {
}

std::string CommandMessage::serialize() const {
    std::ostringstream oss;
    oss << payload_ << '\0' << str2_ << '\0';
    return oss.str();
}

CommandMessage CommandMessage::deserialize(const std::string& data) {
    std::istringstream iss(data);
    std::string payload, str2;
    std::getline(iss, payload, '\0');
    std::getline(iss, str2, '\0');
    return CommandMessage(payload, str2);
}

void CommandMessage::send(asio::ip::tcp::socket& socket) const {
    std::string serialized_data = serialize();
    asio::write(socket, asio::buffer(serialized_data));
}

CommandMessage CommandMessage::receive(asio::ip::tcp::socket& socket) {
    asio::streambuf buffer;
    asio::read_until(socket, buffer, '\0');

    std::istream is(&buffer);
    std::string serialized_data((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());

    CommandMessage data = CommandMessage::deserialize(serialized_data);
    return data;
}

CommandMessage CommandMessage::try_receive(asio::ip::tcp::socket& socket) {
    asio::streambuf buffer;
    asio::error_code error;
    asio::read_until(socket, buffer, '\0', error);

    if (error == asio::error::eof)
        throw EndOfFileException();
    
    if (error)
        throw asio::system_error(error);

    std::istream is(&buffer);
    std::string serialized_data((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());

    CommandMessage data = CommandMessage::deserialize(serialized_data);
    return data;
}
