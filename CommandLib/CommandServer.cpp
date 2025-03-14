#include <cstdio>
#include <iostream>
#include <vector>
#include "CommandLib.h"

using asio::ip::tcp;

CommandServer::CommandServer(tcp::socket&& socket) : m_socket(std::move(socket))
{
    Init();
}

void CommandServer::Init() {
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
}

std::optional<std::string> CommandServer::ReadCommand() {
    asio::streambuf buffer;
    asio::error_code error;
    asio::read_until(m_socket, buffer, '\0', error);

    if (error == asio::error::eof) {
        std::cout << "Connection closed by client" << std::endl;
        return std::nullopt;
    }
    else if (error) {
        throw asio::system_error(error);
    }

    std::string command;
    std::istream is(&buffer);
    std::getline(is, command, '\0');
    return command;
}

void CommandServer::handle_client()
{
    try {
        while (true) {
            auto commandOpt = ReadCommand();
            if (!commandOpt) {
                break;
            }

            std::string command = *commandOpt;
            std::cout << "Received command: " << command << std::endl;

            // Write the command to the child process's STDIN
            DWORD written;
            if (!WriteFile(stdInWrite, command.c_str(), command.size(), &written, NULL)) {
                throw std::runtime_error("Failed to write to child process");
            }

            // Read the output from the child process's STDOUT and STDERR
            auto pipeOutputOpt = ReadPipe();
            if (!pipeOutputOpt) {
                throw std::runtime_error("Failed to read from pipes");
            }

            auto& [stdOutResponse, stdErrResponse] = *pipeOutputOpt;

            std::cout << "StdOut: " << stdOutResponse << std::endl;
            std::cout << "StdError: " << stdErrResponse << std::endl;

            // Create CommandResponse and serialize it
            CommandResponse response{ stdOutResponse };
            std::string serialized_response = response.serialize();

            // Send the serialized response back to the client
            asio::error_code error;
            asio::write(m_socket, asio::buffer(serialized_response), error);
        }
    }
    catch (std::exception& e) {
        std::cerr << "Exception in thread: " << e.what() << std::endl;
    }
}

std::optional<std::pair<std::string, std::string>> CommandServer::ReadPipe() {
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

    if (stdOutResponse.empty() && stdErrResponse.empty()) {
        return std::nullopt;
    }

    return std::make_pair(std::move(stdOutResponse), std::move(stdErrResponse));
}
