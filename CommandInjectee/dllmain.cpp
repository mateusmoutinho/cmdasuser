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

using asio::ip::tcp;
using namespace CommandLib;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID) {
    switch (reason)
    {
    case DLL_PROCESS_ATTACH:
		OutputDebugString(L"CommandInjectee: DLL_PROCESS_ATTACH\n");
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

