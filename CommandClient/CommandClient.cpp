#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <asio.hpp>
#include <CommandLib.h>

// See <repo>/readme.md
using asio::ip::tcp;

int main() {
    try {
        // std::this_thread::sleep_for(std::chrono::seconds(10));

        asio::io_context io_context;
        tcp::resolver resolver(io_context);
        tcp::resolver::results_type endpoints = resolver.resolve("127.0.0.1", "54000");

        tcp::socket socket(io_context);
        asio::connect(socket, endpoints);

        std::string command;
        while (true) {
            std::cout << "Enter command: ";
            std::getline(std::cin, command);

            if (command == "exit") {
                break;
            }

			command += '\0';
            asio::write(socket, asio::buffer(command));
            std::cout << "Command sent: " << command << std::endl;

            // Read response from server
            asio::streambuf response;
            asio::read_until(socket, response, "\0");

            std::istream response_stream(&response);
            std::string serialized_response;
            std::getline(response_stream, serialized_response, '\0');
            
            // Deserialize the response
            CommandResponse command_response = CommandResponse::deserialize(serialized_response);
            std::cout << "Reply from server: " << command_response.StdOut << std::endl;
        }
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
