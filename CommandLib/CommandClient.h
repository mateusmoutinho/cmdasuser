#pragma once

#include <asio.hpp>
#include "CommandMessage.h"
#include "CommandResponse.h"

namespace CommandLib {
    class CommandClient {
        asio::ip::tcp::socket socket_;
		static CommandClient* pSignal_handler;

        static void on_signal_received(int signal);

    public:
        CommandClient(asio::ip::tcp::socket&& socket);

        static const std::string Ctrl_C;

        CommandMessage read_response();
        void send_request(const std::string& command);

		static void set_signal_handler(CommandClient* pSignal_handler) {
			CommandClient::pSignal_handler = pSignal_handler;

			if (pSignal_handler != nullptr) {
				signal(SIGINT, on_signal_received);
			}
			else {
				signal(SIGINT, SIG_DFL);
			}
		}
    };
}

