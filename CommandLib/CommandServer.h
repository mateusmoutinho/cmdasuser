#pragma once

#include <string>
#include <optional>
#include <asio.hpp>
#include "HandleGuard.h"

namespace CommandLib {
    class CommandServer {
        static const std::string eyecatcher_;

        asio::ip::tcp::socket socket_;
        HandleGuard processHandle_, threadHandle_;
        HandleGuard stdInRead_, stdInWrite_, stdOutRead_, stdOutWrite_;

        std::string read_stdout_response();

        void send_eyecatcher();
        void send_delete_eyecatcher();
        void send_response(const std::string& response);
        void process_command(const std::string& command);

    public:
        CommandServer(asio::ip::tcp::socket&& socket);
        void handle_client();
    };
}