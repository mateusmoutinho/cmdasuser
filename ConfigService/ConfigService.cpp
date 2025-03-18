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
#include <lm.h>
#include <ntsecapi.h>
#include <windows.h> // wsock2.h is included in CommandLib.h

#pragma comment(lib, "netapi32.lib")
#pragma comment(lib, "advapi32.lib")

using namespace CommandLib;

class LsaHandleWrapper {
public:
    LsaHandleWrapper(LSA_HANDLE handle) : handle_(handle) {}
    ~LsaHandleWrapper() {
        if (handle_) {
            LsaClose(handle_);
        }
    }
    LSA_HANDLE get() const { return handle_; }
private:
    LSA_HANDLE handle_;
};

class LsaMemoryWrapper {
public:
    LsaMemoryWrapper(void* memory) : memory_(memory) {}
    ~LsaMemoryWrapper() {
        if (memory_) {
            LsaFreeMemory(memory_);
        }
    }
    void* get() const { return memory_; }
private:
    void* memory_;
};

class LocalFreeWrapper {
public:
    LocalFreeWrapper(LPVOID memory) : memory_(memory) {}
    ~LocalFreeWrapper() {
        if (memory_) {
            LocalFree(memory_);
        }
    }
    LPVOID get() const { return memory_; }
private:
    LPVOID memory_;
};

void DisplayError(NTSTATUS status) {
    DWORD winError = LsaNtStatusToWinError(status);
    LPVOID errorMsg;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        winError,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPWSTR)&errorMsg,
        0,
        NULL
    );
    LocalFreeWrapper errorMsgWrapper(errorMsg);
    std::wcerr << L"Error: " << (LPWSTR)errorMsg << std::endl;
}

void DisplayUsersWithLogonAsServiceRight() {
    LSA_OBJECT_ATTRIBUTES objectAttributes;
    ZeroMemory(&objectAttributes, sizeof(objectAttributes));

    LSA_HANDLE policyHandle;
    if (LsaOpenPolicy(NULL, &objectAttributes, POLICY_LOOKUP_NAMES | POLICY_VIEW_LOCAL_INFORMATION, &policyHandle) != 0) {
        std::cerr << "Failed to open policy handle." << std::endl;
        return;
    }
    LsaHandleWrapper policyHandleWrapper(policyHandle);

    LSA_UNICODE_STRING userRightsString;
    std::wstring right = L"SeServiceLogonRight";
    userRightsString.Buffer = const_cast<wchar_t*>(right.c_str());
    userRightsString.Length = static_cast<USHORT>(right.size() * sizeof(wchar_t));
    userRightsString.MaximumLength = static_cast<USHORT>((right.size() + 1) * sizeof(wchar_t));

    LSA_ENUMERATION_HANDLE enumHandle = 0;
    PLSA_ENUMERATION_INFORMATION enumBuffer = nullptr;
    ULONG countReturned = 0;

    NTSTATUS status = LsaEnumerateAccountsWithUserRight(policyHandle, &userRightsString, (void**)&enumBuffer, &countReturned);
    if (status != 0) {
        DisplayError(status);
        return;
    }
    LsaMemoryWrapper enumBufferWrapper(enumBuffer);

    for (ULONG i = 0; i < countReturned; ++i) {
        LSA_TRANSLATED_NAME* translatedName = nullptr;
        LSA_REFERENCED_DOMAIN_LIST* domainList = nullptr;
        if (LsaLookupSids(policyHandle, 1, &enumBuffer[i].Sid, &domainList, &translatedName) == 0) {
            LsaMemoryWrapper translatedNameWrapper(translatedName);
            LsaMemoryWrapper domainListWrapper(domainList);
            std::wcout << translatedName->Name.Buffer << std::endl;
        }
    }
}

int main() {
    try {
        DisplayUsersWithLogonAsServiceRight();
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
