#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <asio.hpp>
#include <CommandLib.h>

using asio::ip::tcp;
using namespace CommandLib;

std::string get_command_input() {
    std::string command;
    while (true) {
        std::getline(std::cin, command);

        if (!std::cin.fail() || std::cin.eof()) {
            break;
        }

        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
        /*else 
            break;*/

        /*else if (std::cin.eof()) {
            break;
        }*/
    }
    return command;

}

int main() {
    try {
        // std::this_thread::sleep_for(std::chrono::seconds(10));

        asio::io_context io_context;
        tcp::resolver resolver(io_context);
        tcp::resolver::results_type endpoints = resolver.resolve("127.0.0.1", "54000");

        tcp::socket socket(io_context);
        asio::connect(socket, endpoints);

		CommandClient client(io_context, std::move(socket));
        CommandClient::set_signal_handler(&client);

        while (true) {
            CommandMessage command_response = client.read_response();
            // std::cout << "Server says: " << command_response.get_payload() << std::endl;
            // std::cout << "Server sent: " << command_response.get_payload().size() << " bytes" << std::endl;
            // std::cout << "Enter command: ";

            std::cout << command_response.get_payload();

            std::string command;
			while (command.empty())
				command = get_command_input();

            if (command == "exit")
                break;

			client.send_request(command);
        }

        CommandClient::set_signal_handler(nullptr);
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
