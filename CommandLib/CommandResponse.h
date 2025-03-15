#pragma once

#include <string>

namespace CommandLib {
    struct CommandResponse {
        const std::string StdOut;
        CommandResponse(const std::string& stdOut) : StdOut(stdOut) {}

        std::string serialize() const;
        static CommandResponse deserialize(const std::string& data);
    };
}
