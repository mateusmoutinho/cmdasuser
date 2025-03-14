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

class CommandServer {
private:
    asio::ip::tcp::socket socket_;
    HandleGuard processHandle_, threadHandle_;
    HandleGuard stdInRead_, stdInWrite_, stdOutRead_, stdErrRead_, stdOutWrite_, stdErrWrite_;

    void init();
    void send_request(const std::string& command);
    void send_response(std::pair<std::string, std::string> response);

    std::optional<std::string> read_request();
    std::optional<std::pair<std::string, std::string>> read_response();


public:
    CommandServer(asio::ip::tcp::socket&& socket);
    void handle_client();
};
