#include "CommandLib.h"

std::string CommandResponse::serialize() const {
    return StdOut + "\0";
}

CommandResponse CommandResponse::deserialize(const std::string& data) {
    CommandResponse response;
    response.StdOut = data;
    return response;
}