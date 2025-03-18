#pragma once

#include <Windows.h>

namespace CommandLib {
    class VirtualMemoryGuard {
        HANDLE hProcess_;
        LPVOID pMemory_;

    public:
        VirtualMemoryGuard(HANDLE hProcess, LPVOID pMemory)
            : hProcess_(hProcess), pMemory_(pMemory) {
        }

        ~VirtualMemoryGuard() {
            if (pMemory_ != nullptr) {
                VirtualFreeEx(hProcess_, pMemory_, 0, MEM_RELEASE);
            }
        }

        LPVOID get() const { return pMemory_; }

        // Disable copy semantics
        VirtualMemoryGuard(const VirtualMemoryGuard&) = delete;
        VirtualMemoryGuard& operator=(const VirtualMemoryGuard&) = delete;

        // Enable move semantics
        VirtualMemoryGuard(VirtualMemoryGuard&& other) noexcept
            : hProcess_(other.hProcess_), pMemory_(other.pMemory_) {
            other.pMemory_ = nullptr;
        }

        VirtualMemoryGuard& operator=(VirtualMemoryGuard&& other) noexcept {
            if (this != &other) {
                if (pMemory_ != nullptr) {
                    VirtualFreeEx(hProcess_, pMemory_, 0, MEM_RELEASE);
                }
                hProcess_ = other.hProcess_;
                pMemory_ = other.pMemory_;
                other.pMemory_ = nullptr;
            }
            return *this;
        }
    };
}
