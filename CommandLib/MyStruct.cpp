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
