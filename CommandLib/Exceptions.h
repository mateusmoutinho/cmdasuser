#pragma once
#include <stdexcept>

namespace CommandLib {
    class EndOfFileException : public std::runtime_error {
    public:
        EndOfFileException() : std::runtime_error("End of file reached") {}
    };
}
