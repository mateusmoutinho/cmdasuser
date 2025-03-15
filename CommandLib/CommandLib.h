#pragma once

#include <string>
#include <optional>
#include <asio.hpp>
#include <Windows.h>

#include "HandleGuard.h"

struct CommandResponse {
    const std::string StdOut;
    CommandResponse(const std::string& stdOut) : StdOut(stdOut) {}

    std::string serialize() const;
    static CommandResponse deserialize(const std::string& data);
};

class CommandClient {
    asio::ip::tcp::socket socket_;
public:
    CommandClient(asio::ip::tcp::socket&& socket);

    CommandResponse read_response();
	void send_request(std::string&& command);
};

class CommandServer {
    asio::ip::tcp::socket socket_;
    HandleGuard processHandle_, threadHandle_;
    HandleGuard stdInRead_, stdInWrite_, stdOutRead_, stdOutWrite_;

    void init();
    void init_pipes();
    void process_request(std::string&& command);
    void send_response(const std::string & response);

    std::optional<std::string> read_request();
    std::optional<std::string> read_response();
    std::optional<std::string> read_response(int max_retries, DWORD sleep_interval_ms);

public:
    CommandServer(asio::ip::tcp::socket&& socket);
    void handle_client();
};
