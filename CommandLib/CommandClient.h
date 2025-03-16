#pragma once

#include <asio.hpp>
#include "CommandMessage.h"
#include "CommandResponse.h"

namespace CommandLib {
    class CommandClient {
		asio::signal_set signals_;
        asio::ip::tcp::socket socket_;

		static CommandClient* pSignal_handler_;
        static void on_signal_received(int signal);
		void handle_signal(const asio::error_code& error, int signal_number);

    public:
        static const std::string Ctrl_C;

        CommandClient(asio::io_context& io_context, asio::ip::tcp::socket&&);
        CommandMessage read_response();
        void send_request(const std::string& command);

		static void set_signal_handler(CommandClient* pSignal_handler) {
			CommandClient::pSignal_handler_ = pSignal_handler;

			if (pSignal_handler != nullptr) {
				signal(SIGINT, on_signal_received);
			}
			else {
				signal(SIGINT, SIG_DFL);
			}
		}
    };
}

