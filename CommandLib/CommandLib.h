#pragma once

#include <string>
#include <Windows.h>

struct CommandResponse {
    std::string StdOut;
    std::string serialize() const;
    static CommandResponse deserialize(const std::string& data);
};

class HandleGuard {
public:
    explicit HandleGuard(HANDLE handle = nullptr) : handle_(handle) {}

    ~HandleGuard() {
        if (handle_ != nullptr && handle_ != INVALID_HANDLE_VALUE) {
            CloseHandle(handle_);
        }
    }

    // Disable copy semantics
    HandleGuard(const HandleGuard&) = delete;
    HandleGuard& operator=(const HandleGuard&) = delete;

    // Enable move semantics
    HandleGuard(HandleGuard&& other) noexcept : handle_(other.handle_) {
        other.handle_ = nullptr;
    }

    HandleGuard& operator=(HandleGuard&& other) noexcept {
        if (this != &other) {
            if (handle_ != nullptr && handle_ != INVALID_HANDLE_VALUE) {
                CloseHandle(handle_);
            }
            handle_ = other.handle_;
            other.handle_ = nullptr;
        }
        return *this;
    }

    HANDLE get() const {
        return handle_;
    }

    HANDLE* get_pointer() {
        return &handle_;
    }

    void reset(HANDLE handle = nullptr) {
        if (handle_ != nullptr && handle_ != INVALID_HANDLE_VALUE) {
            CloseHandle(handle_);
        }
        handle_ = handle;
    }

    operator HANDLE() const {
        return handle_;
    }

private:
    HANDLE handle_;
};
