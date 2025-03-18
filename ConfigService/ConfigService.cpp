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
#include <sddl.h>

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

void AddLogonAsServiceRight(const std::wstring& accountName) {
    LSA_OBJECT_ATTRIBUTES objectAttributes;
    ZeroMemory(&objectAttributes, sizeof(objectAttributes));

    LSA_HANDLE policyHandle;
    NTSTATUS status = LsaOpenPolicy(NULL, &objectAttributes, POLICY_LOOKUP_NAMES | POLICY_CREATE_ACCOUNT, &policyHandle);
    if (status != 0) {
        DisplayError(status);
        return;
    }
    LsaHandleWrapper policyHandleWrapper(policyHandle);

    LSA_UNICODE_STRING userRightsString;
    std::wstring right = L"SeServiceLogonRight";
    userRightsString.Buffer = const_cast<wchar_t*>(right.c_str());
    userRightsString.Length = static_cast<USHORT>(right.size() * sizeof(wchar_t));
    userRightsString.MaximumLength = static_cast<USHORT>((right.size() + 1) * sizeof(wchar_t));

    PSID accountSid = nullptr;
    SID_NAME_USE sidType;
    DWORD sidSize = 0;
    DWORD domainSize = 0;
    LookupAccountNameW(NULL, accountName.c_str(), accountSid, &sidSize, NULL, &domainSize, &sidType);

    std::vector<BYTE> sidBuffer(sidSize);
    std::vector<wchar_t> domainBuffer(domainSize);
    accountSid = reinterpret_cast<PSID>(sidBuffer.data());

    if (!LookupAccountNameW(NULL, accountName.c_str(), accountSid, &sidSize, domainBuffer.data(), &domainSize, &sidType)) {
        std::cerr << "Failed to lookup account name." << std::endl;
        return;
    }

    LPWSTR sidString = nullptr;
    if (ConvertSidToStringSidW(accountSid, &sidString)) {
        std::wcout << L"SID for account " << accountName << L": " << sidString << std::endl;
        LocalFree(sidString);
    }
    else {
        std::cerr << "Failed to convert SID to string." << std::endl;
    }

    status = LsaAddAccountRights(policyHandle, accountSid, &userRightsString, 1);
    if (status != 0) {
        DisplayError(status);
        return;
    }

    std::wcout << L"Successfully added 'Logon as a service' right to account: " << accountName << std::endl;
}

int wmain(int argc, wchar_t* argv[]) {
    if (argc < 2) {
        std::wcerr << L"Usage: " << argv[0] << L" <command> [<AccountName>]" << std::endl;
        std::wcerr << L"Commands:" << std::endl;
        std::wcerr << L"  add <AccountName>   Add 'Logon as a service' right to the specified account" << std::endl;
        std::wcerr << L"  list                Display users with 'Logon as a service' right" << std::endl;
        std::wcerr << L"Examples:" << std::endl;
        std::wcerr << L"  " << argv[0] << L" add LocalUser" << std::endl;
        std::wcerr << L"  " << argv[0] << L" add DOMAIN\\DomainUser" << std::endl;
        std::wcerr << L"  " << argv[0] << L" add LocalGroup" << std::endl;
        std::wcerr << L"  " << argv[0] << L" add DOMAIN\\DomainGroup" << std::endl;
        std::wcerr << L"  " << argv[0] << L" list" << std::endl;
        return 1;
    }

    std::wstring command = argv[1];

    try {
        if (command == L"add") {
            if (argc != 3) {
                std::wcerr << L"Usage: " << argv[0] << L" add <AccountName>" << std::endl;
                return 1;
            }
            std::wstring accountName = argv[2];
            AddLogonAsServiceRight(accountName);
        }
        else if (command == L"list") {
            DisplayUsersWithLogonAsServiceRight();
        }
        else {
            std::wcerr << L"Unknown command: " << command << std::endl;
            return 1;
        }
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
