#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <asio.hpp>
#include <CommandLib.h>

using asio::ip::tcp;
using namespace CommandLib;

int main() {
    try {
        // std::this_thread::sleep_for(std::chrono::seconds(10));

        asio::io_context io_context;
        tcp::resolver resolver(io_context);
        tcp::resolver::results_type endpoints = resolver.resolve("127.0.0.1", "54000");

        tcp::socket socket(io_context);
        asio::connect(socket, endpoints);

        std::string command;
		CommandClient client(std::move(socket));

        while (true) {
            CommandMessage command_response = client.read_response();
            // std::cout << "Server says: " << command_response.get_payload() << std::endl;
            // std::cout << "Server sent: " << command_response.get_payload().size() << " bytes" << std::endl;
            // std::cout << "Enter command: ";

            std::cout << command_response.get_payload();
            std::getline(std::cin, command);

            if (command == "exit")
                break;
            
			client.send_request(std::move(command));
        }
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
