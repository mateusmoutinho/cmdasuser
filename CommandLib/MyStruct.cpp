#include <iostream>
#include <asio.hpp>
#include "MyStruct.h"

using asio::ip::tcp;

MyStruct::MyStruct(const std::string& str1, const std::string& str2) : str1_(str1), str2_(str2) {
}

std::string MyStruct::serialize() const {
    std::ostringstream oss;
    oss << str1_ << '\0' << str2_ << '\0';
    return oss.str();
}

MyStruct MyStruct::deserialize(const std::string& data) {
    std::istringstream iss(data);
    std::string str1, str2;
    std::getline(iss, str1, '\0');
    std::getline(iss, str2, '\0');
    return MyStruct(str1, str2);
}

void MyStruct::send(asio::ip::tcp::socket& socket) const {
    std::string serialized_data = serialize();
    asio::write(socket, asio::buffer(serialized_data));
}

MyStruct receive(asio::ip::tcp::socket& socket) {
    asio::streambuf buffer;
    asio::read_until(socket, buffer, '\0');

    std::istream is(&buffer);
    std::string serialized_data((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());
    MyStruct data = MyStruct::deserialize(serialized_data);
    return data;
}
