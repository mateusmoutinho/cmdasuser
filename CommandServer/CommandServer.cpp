#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <array>
#include <memory>
#include <cstdio>
#include <asio.hpp>
#include <CommandLib.h>
#include <tchar.h>
#include <windows.h> // wsock2.h is included in CommandLib.h

#ifdef _WIN32
#define popen _popen
#define pclose _pclose
#endif

// See <repo>/readme.md
using asio::ip::tcp;

void handle_client(tcp::socket socket) {
    try {
        HandleGuard processHandle, threadHandle;
        HandleGuard stdInRead, stdInWrite, stdOutRead, stdErrRead, stdOutWrite, stdErrWrite;

        // Create pipes for the child process's STDIN and STDOUT
        SECURITY_ATTRIBUTES saAttr = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };

        if (!CreatePipe(stdInRead.get_pointer(), stdInWrite.get_pointer(), &saAttr, 0) ||
            !SetHandleInformation(stdInWrite, HANDLE_FLAG_INHERIT, 0) ||
            !CreatePipe(stdOutRead.get_pointer(), stdOutWrite.get_pointer(), &saAttr, 0) ||
            !SetHandleInformation(stdOutRead, HANDLE_FLAG_INHERIT, 0) || 
            !CreatePipe(stdErrRead.get_pointer(), stdErrWrite.get_pointer(), &saAttr, 0) ||
            !SetHandleInformation(stdErrRead, HANDLE_FLAG_INHERIT, 0)) {
            throw std::runtime_error("Failed to create pipes");
        }

        // Set up the start info struct
        STARTUPINFO siStartInfo = { sizeof(STARTUPINFO) };
        siStartInfo.hStdInput = stdInRead;
        siStartInfo.hStdError = stdErrWrite;
        siStartInfo.hStdOutput = stdOutWrite;
        siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

        // Create the child process
        PROCESS_INFORMATION piProcInfo;

        std::string cmd = "cmd.exe";
		std::vector<TCHAR> command_line;
        command_line.assign(cmd.begin(), cmd.end());
        command_line.push_back('\0'); // Add null terminator

        if (!CreateProcess(NULL, &command_line[0], NULL, NULL, TRUE, 0, NULL, NULL, &siStartInfo, &piProcInfo)) {
            throw std::runtime_error("Failed to create process");
        }

        processHandle.reset(piProcInfo.hProcess);
        threadHandle.reset(piProcInfo.hThread);

        asio::streambuf buffer;
        asio::error_code error;

        while (true) {
            asio::read_until(socket, buffer, '\0', error);

            if (error == asio::error::eof) {
                std::cout << "Connection closed by client" << std::endl;
                break;
            }
            else if (error) {
                throw asio::system_error(error);
            }

            std::string command;
            std::istream is(&buffer);
            std::getline(is, command, '\0');
            std::cout << "Received command: " << command << std::endl;

            // Write the command to the child process's STDIN
            DWORD written;
            if (!WriteFile(stdInWrite, command.c_str(), command.size(), &written, NULL)) {
                throw std::runtime_error("Failed to write to child process");
            }

            // Read the output from the child process's STDOUT
            DWORD read;
            std::string stdErrResponse;
            std::string stdOutResponse;
            std::vector<char> output_buffer(1024 * 10);

            DWORD stdErrBytes = 1;
            DWORD stdOutBytes = 1;
            while (stdErrBytes != 0 || stdOutBytes != 0) {
                if (!PeekNamedPipe(stdOutRead, NULL, 0, NULL, &stdOutBytes, NULL)) {
                    throw std::runtime_error("Failed to peek stdout pipe");
                }

                if (stdOutBytes != 0) {
					if (!ReadFile(stdOutRead, output_buffer.data(), output_buffer.size(), &read, NULL))
						stdOutBytes = 0;
					else
                    {
                        if (read != 0)
                            stdOutResponse.append(output_buffer.data(), read);
                    }
                }

                if (!PeekNamedPipe(stdErrRead, NULL, 0, NULL, &stdErrBytes, NULL)) {
                    throw std::runtime_error("Failed to peek stderr pipe");
                }

                if (stdErrBytes != 0) {
                    if (!ReadFile(stdErrRead, output_buffer.data(), output_buffer.size(), &read, NULL))
                        stdErrBytes = 0;
                    else
                    {
                        if (read != 0)
                            stdErrResponse.append(output_buffer.data(), read);
                    }
                }
            }

			std::cout << "StdOut: " << stdOutResponse << std::endl;
			std::cout << "StdError: " << stdErrResponse << std::endl;

            // Create CommandResponse and serialize it
            CommandResponse response{ stdOutResponse };
            std::string serialized_response = response.serialize();

            // Send the serialized response back to the client
            asio::write(socket, asio::buffer(serialized_response), error);
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
