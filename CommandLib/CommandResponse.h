#pragma once

#include <string>

namespace CommandLib {
    struct CommandResponse2 {
        const std::string StdOut;
        CommandResponse2(const std::string& stdOut) : StdOut(stdOut) {}

        std::string serialize() const;
        static CommandResponse2 deserialize(const std::string& data);
    };
}
