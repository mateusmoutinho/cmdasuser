#include <iostream>
#include <string>
#include <asio.hpp>
#include <thread>
#include <chrono>

// See <repo>/readme.md
using asio::ip::tcp;

int main() {
    try {
        std::this_thread::sleep_for(std::chrono::seconds(10));

        asio::io_context io_context;
        tcp::resolver resolver(io_context);
        tcp::resolver::results_type endpoints = resolver.resolve("127.0.0.1", "54000");

        tcp::socket socket(io_context);
        asio::connect(socket, endpoints);

        std::string command = "echo Hello, Server!";
        asio::write(socket, asio::buffer(command));

        std::cout << "Command sent: " << command << std::endl;
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
