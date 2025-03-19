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
#include <tlhelp32.h> // For thread enumeration
#include "HandleGuard.h"
#include "VirtualMemoryGuard.h"

using asio::ip::tcp;
using namespace CommandLib;

HMODULE g_hModule = NULL;

void WriteLog(const std::wstring& message) {
    wchar_t dllPath[MAX_PATH];
    if (GetModuleFileNameW(g_hModule, dllPath, MAX_PATH) == 0) {
        return;
    }

    std::wstring::size_type pos = std::wstring(dllPath).find_last_of(L"\\/");
    std::wstring logFilePath = std::wstring(dllPath).substr(0, pos) + L"\\marty_log.txt";

	// std::wstring logMessage = L"WriteLog: " + logFilePath + L"\n";
    // OutputDebugString(logMessage.c_str());

    HandleGuard logFile(CreateFileW(logFilePath.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, 
        OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL));

    if (logFile == INVALID_HANDLE_VALUE) {
        OutputDebugString(L"WriteLog: FAILED\n");
        return;
    }

    DWORD bytesWritten;
    SetFilePointer(logFile, 0, NULL, FILE_END);
    WriteFile(logFile, message.c_str(), static_cast<DWORD>(message.size() * sizeof(wchar_t)), &bytesWritten, NULL);
    FlushFileBuffers(logFile);
}

void SuspendAllThreadsExceptCurrent() {
    WriteLog(L"SuspendAllThreadsExceptCurrent: Start\n");

    DWORD currentThreadId = GetCurrentThreadId();
    HandleGuard snapshot(CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0));
    if (snapshot == INVALID_HANDLE_VALUE) {
        WriteLog(L"SuspendAllThreadsExceptCurrent: Failed to create snapshot\n");
        return;
    }

    THREADENTRY32 threadEntry;
    threadEntry.dwSize = sizeof(THREADENTRY32);

    if (Thread32First(snapshot, &threadEntry)) {
        do {
            if (threadEntry.th32OwnerProcessID == GetCurrentProcessId() && threadEntry.th32ThreadID != currentThreadId) {
                HandleGuard threadHandle(OpenThread(THREAD_SUSPEND_RESUME, FALSE, threadEntry.th32ThreadID));
                if (threadHandle != NULL) {
                    DWORD result = SuspendThread(threadHandle);

                    WriteLog(L"Suspended thread (tid:" + 
                        std::to_wstring(threadEntry.th32ThreadID) + L"): " + 
                        std::to_wstring(result) + 
                        L"\n");
                }
            }
        } while (Thread32Next(snapshot, &threadEntry));
    }

    WriteLog(L"SuspendAllThreadsExceptCurrent: End\n");
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID) {
    switch (reason)
    {
    case DLL_PROCESS_ATTACH:
        g_hModule = hModule;
		OutputDebugString(L"CommandInjectee: DLL_PROCESS_ATTACH\n");
        WriteLog(L"CommandInjectee: DLL_PROCESS_ATTACH\n");
		SuspendAllThreadsExceptCurrent();
		break;

    case DLL_PROCESS_DETACH:
        WriteLog(L"CommandInjectee: DLL_PROCESS_DETACH\n");
        break;
    }
    return TRUE;
}

