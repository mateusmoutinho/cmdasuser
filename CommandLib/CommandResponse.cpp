#include "CommandResponse.h"

using namespace CommandLib;

std::string CommandResponse::serialize() const {
    return StdOut + "\0";
}

CommandResponse CommandResponse::deserialize(const std::string& data) {
    return CommandResponse(data);
}
