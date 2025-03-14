#pragma once

#include <string>

struct CommandResponse {
    std::string StdOut;
    std::string serialize() const;
    static CommandResponse deserialize(const std::string& data);
};
