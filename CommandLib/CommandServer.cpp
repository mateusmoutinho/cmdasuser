#include <cstdio>
#include <iostream>
#include <vector>
#include "CommandLib.h"

using asio::ip::tcp;

CommandServer::CommandServer(tcp::socket&& socket) : socket_(std::move(socket))
{
    init();
}

void CommandServer::init() {
    SECURITY_ATTRIBUTES saAttr = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };

    // Create pipes for the child process's STDIN and STDOUT
    if (!CreatePipe(stdInRead_.get_pointer(), stdInWrite_.get_pointer(), &saAttr, 0) ||
        !SetHandleInformation(stdInWrite_, HANDLE_FLAG_INHERIT, 0) ||
        !CreatePipe(stdOutRead_.get_pointer(), stdOutWrite_.get_pointer(), &saAttr, 0) ||
        !SetHandleInformation(stdOutRead_, HANDLE_FLAG_INHERIT, 0) ||
        !CreatePipe(stdErrRead_.get_pointer(), stdErrWrite_.get_pointer(), &saAttr, 0) ||
        !SetHandleInformation(stdErrRead_, HANDLE_FLAG_INHERIT, 0)) {
        throw std::runtime_error("Failed to create pipes");
    }

    STARTUPINFO startupInfo = { sizeof(STARTUPINFO) };
    startupInfo.hStdInput = stdInRead_;
    startupInfo.hStdError = stdErrWrite_;
    startupInfo.hStdOutput = stdOutWrite_;
    startupInfo.dwFlags |= STARTF_USESTDHANDLES;

    std::string cmd = "cmd.exe";
    std::vector<TCHAR> command_line;
    command_line.assign(cmd.begin(), cmd.end());
    command_line.push_back('\0'); // Add null terminator

    PROCESS_INFORMATION procInfo;
    if (!CreateProcess(NULL, &command_line[0], NULL, NULL, TRUE, 0, NULL, NULL, &startupInfo, &procInfo)) {
        throw std::runtime_error("Failed to create process");
    }

    threadHandle_.reset(procInfo.hThread);
    processHandle_.reset(procInfo.hProcess);
}

std::optional<std::string> CommandServer::read_request() {
    asio::streambuf buffer;
    asio::error_code error;
    asio::read_until(socket_, buffer, '\0', error);

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

void CommandServer::handle_client() {
    while (true) {
        auto response = read_response();
        if (!response) {
            throw std::runtime_error("Failed to read response");
        }
        send_response(*response);

        auto request = read_request();
        if (!request) {
            break;
        }
		process_request(*request);
    }
}

void CommandServer::send_response(std::pair<std::string, std::string> response)
{
    auto& [stdOutResponse, stdErrResponse] = response;
    std::cout << "StdOut: " << stdOutResponse << std::endl;
    std::cout << "StdError: " << stdErrResponse << std::endl;

    CommandResponse commandResponse{ stdOutResponse };
    std::string serialized_response = commandResponse.serialize();

    asio::error_code error;
    asio::write(socket_, asio::buffer(serialized_response), error);
}

void CommandServer::process_request(const std::string& command)
{
    std::cout << "Received command: " << command << std::endl;

    // Write the command to the child process's STDIN
    DWORD written;
    if (!WriteFile(stdInWrite_, command.c_str(), command.size(), &written, NULL)) {
        throw std::runtime_error("Failed to write to child process");
    }
}

std::optional<std::pair<std::string, std::string>> CommandServer::read_response() {
    DWORD read;
    std::string stdErrResponse;
    std::string stdOutResponse;
    std::vector<char> output_buffer(1024 * 10);

    DWORD stdErrBytes = 1;
    DWORD stdOutBytes = 1;
    while (stdErrBytes != 0 || stdOutBytes != 0) {
        if (!PeekNamedPipe(stdOutRead_, NULL, 0, NULL, &stdOutBytes, NULL)) {
            throw std::runtime_error("Failed to peek stdout pipe");
        }

        if (stdOutBytes != 0) {
            if (!ReadFile(stdOutRead_, output_buffer.data(), output_buffer.size(), &read, NULL))
                stdOutBytes = 0;
            else
            {
                if (read != 0)
                    stdOutResponse.append(output_buffer.data(), read);
            }
        }

        if (!PeekNamedPipe(stdErrRead_, NULL, 0, NULL, &stdErrBytes, NULL)) {
            throw std::runtime_error("Failed to peek stderr pipe");
        }

        if (stdErrBytes != 0) {
            if (!ReadFile(stdErrRead_, output_buffer.data(), output_buffer.size(), &read, NULL))
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
