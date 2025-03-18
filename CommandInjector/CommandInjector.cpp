#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <array>
#include <memory>
#include <cstdio>
#include <utility>
#include <optional>
#include <asio.hpp>
#include <CommandLib.h>
#include <tchar.h>
#include <windows.h> // wsock2.h is included in CommandLib.h
#include <stdexcept>
#include <filesystem>
#include "HandleGuard.h"
#include "VirtualMemoryGuard.h"

using asio::ip::tcp;
using namespace CommandLib;
namespace fs = std::filesystem;

bool InjectDLL(DWORD processID, const char* dllPath) {
    HandleGuard hProcess(OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID));
    if (hProcess == nullptr) {
        std::cerr << "Failed to open target process." << std::endl;
        return false;
    }

    LPVOID pDllPath = VirtualAllocEx(hProcess, NULL, strlen(dllPath) + 1, MEM_COMMIT, PAGE_READWRITE);
    if (pDllPath == NULL) {
        std::cerr << "Failed to allocate memory in target process." << std::endl;
        return false;
    }

    VirtualMemoryGuard memoryGuard(hProcess, pDllPath);

    if (!WriteProcessMemory(hProcess, pDllPath, (LPVOID)dllPath, strlen(dllPath) + 1, NULL)) {
        std::cerr << "Failed to write DLL path to target process memory." << std::endl;
        return false;
    }

    HandleGuard hThread(CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, pDllPath, 0, NULL));
    if (hThread == nullptr) {
        std::cerr << "Failed to create remote thread in target process." << std::endl;
        return false;
    }

    WaitForSingleObject(hThread, INFINITE);
    return true;
}

std::string GetAbsolutePath(const std::string& dllPath) {
    if (fs::path(dllPath).is_absolute()) {
        return dllPath;
    }

    // Check current directory
    fs::path currentPath = fs::current_path() / dllPath;
    if (fs::exists(currentPath)) {
        return currentPath.string();
    }

    // Check the directory of the executing binary
    TCHAR exePath[MAX_PATH];
    if (GetModuleFileName(NULL, exePath, MAX_PATH)) {
        fs::path exeDir = fs::path(exePath).parent_path();
        fs::path exeDirPath = exeDir / dllPath;
        if (fs::exists(exeDirPath)) {
            return exeDirPath.string();
        }
    }

    // Check system directories
    TCHAR systemPath[MAX_PATH];
    if (GetSystemDirectory(systemPath, MAX_PATH)) {
        fs::path sysPath = fs::path(systemPath) / dllPath;
        if (fs::exists(sysPath)) {
            return sysPath.string();
        }
    }

    throw std::runtime_error("DLL not found: " + dllPath);
}

int main(int argc, char* argv[]) {
    if (argc < 2 || argc > 3) {
        std::cerr << "Usage: " << argv[0] << " <process_id> [dll_path]" << std::endl;
        return 1;
    }

    DWORD processID = std::stoul(argv[1]);
    std::string dllPath = (argc == 3) ? argv[2] : "CommandInjectee.dll";

    try {
        std::string absoluteDllPath = GetAbsolutePath(dllPath);
		std::cout << "Injecting DLL: " << absoluteDllPath << std::endl;

        if (InjectDLL(processID, absoluteDllPath.c_str())) {
            std::cout << "DLL injected successfully." << std::endl;
        }
        else {
            std::cerr << "DLL injection failed." << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
