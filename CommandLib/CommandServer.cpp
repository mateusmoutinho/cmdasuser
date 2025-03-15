#include <cstdio>
#include <iostream>
#include <vector>
#include "CommandLib.h"

using asio::ip::tcp;

const std::string CommandServer::marker_ = "MartWasHere";

CommandServer::CommandServer(tcp::socket&& socket) : socket_(std::move(socket))
{
    init();
}

void CommandServer::init() {

    init_pipes();

    STARTUPINFO startupInfo = { sizeof(STARTUPINFO) };
    startupInfo.hStdInput = stdInRead_;
    startupInfo.hStdError = stdOutWrite_;
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

void CommandServer::init_pipes()
{
    SECURITY_ATTRIBUTES saAttr = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };

    /// Create pipes for the child process's STDIN and STDOUT
    if (!CreatePipe(stdInRead_.get_pointer(), stdInWrite_.get_pointer(), &saAttr, 0) ||
        !SetHandleInformation(stdInWrite_, HANDLE_FLAG_INHERIT, 0) ||
        !CreatePipe(stdOutRead_.get_pointer(), stdOutWrite_.get_pointer(), &saAttr, 0) ||
        !SetHandleInformation(stdOutRead_, HANDLE_FLAG_INHERIT, 0)) {
        throw std::runtime_error("Failed to create pipes");
    }
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

    // We use this to find the end of the command output.
	const std::string echo_marker = "ECHO " + marker_; 
    process_command(echo_marker);

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
		process_command(*request);
        process_command(echo_marker);
    }
}

void CommandServer::send_response(const std::string & response)
{
    std::cout << "Server says: " << response << std::endl;

    CommandResponse commandResponse{ response };
    std::string serialized_response = commandResponse.serialize();

    asio::error_code error;
    asio::write(socket_, asio::buffer(serialized_response), error);
}

void CommandServer::process_command(const std::string & command)
{
    std::cout << "Processing command: " << command << std::endl;
	std::string commandLine = command + "\n"; // signal cmd.exe to process it

    // Write the command to the cmd.exe process's STDIN
    DWORD written;
    if (!WriteFile(stdInWrite_, commandLine.c_str(), commandLine.size(), &written, NULL)) {
        throw std::runtime_error("Failed to write to child process");
    }
}

std::optional<std::string> CommandServer::read_response() {
    DWORD read;
    std::string stdOutResponse;
    std::vector<char> output_buffer(1024 * 10);

    while (true) {
        DWORD stdOutBytes = 0;
        if (!PeekNamedPipe(stdOutRead_, NULL, 0, NULL, &stdOutBytes, NULL)) {
            throw std::runtime_error("Failed to peek stdout pipe");
        }

        if (stdOutBytes != 0) {
            if (!ReadFile(stdOutRead_, output_buffer.data(), output_buffer.size(), &read, NULL)) {
                throw std::runtime_error("Failed to read stdout pipe");
            }
            if (read > 0) {
                stdOutResponse.append(output_buffer.data(), read);
                if (stdOutResponse.find(marker_) != std::string::npos) {
                    break;
                }
            }
        }
        else {
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Sleep for a short interval before retrying
        }
    }

    // Remove the marker from the response
    size_t marker_pos = stdOutResponse.find(marker_);
    if (marker_pos != std::string::npos) {
        stdOutResponse.resize(marker_pos);
    }

    if (stdOutResponse.empty()) {
        return std::nullopt;
    }

    return stdOutResponse;
}
