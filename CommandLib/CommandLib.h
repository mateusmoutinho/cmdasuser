#pragma once

#include <string>
#include <optional>
#include <asio.hpp>
#include <Windows.h>

#include "HandleGuard.h"

struct CommandResponse {
    std::string StdOut;
    std::string serialize() const;
    static CommandResponse deserialize(const std::string& data);
};

class CommandServer {
private:
    asio::ip::tcp::socket socket_;
    HandleGuard processHandle_, threadHandle_;
    HandleGuard stdInRead_, stdInWrite_, stdOutRead_, stdErrRead_, stdOutWrite_, stdErrWrite_;

    void Init();
    std::optional<std::string> ReadCommand();
    std::optional<std::pair<std::string, std::string>> ReadPipe();

public:
    CommandServer(asio::ip::tcp::socket&& socket);
    void handle_client();
};
