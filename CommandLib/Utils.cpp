#include <Windows.h>
#include <sddl.h>
#include <Lmcons.h>
#include <iostream>
#include <string>
#include <vector>
#include "Utils.h"

using namespace CommandLib;

std::string CommandLib::GetCurrentSid() {
    HANDLE tokenHandle = nullptr;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &tokenHandle)) {
        std::cerr << "Failed to open process token." << std::endl;
        return "";
    }

    DWORD tokenInfoLength = 0;
    GetTokenInformation(tokenHandle, TokenUser, nullptr, 0, &tokenInfoLength);
    if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
        std::cerr << "Failed to get token information length." << std::endl;
        CloseHandle(tokenHandle);
        return "";
    }

    std::vector<BYTE> tokenInfo(tokenInfoLength);
    if (!GetTokenInformation(tokenHandle, TokenUser, tokenInfo.data(), tokenInfoLength, &tokenInfoLength)) {
        std::cerr << "Failed to get token information." << std::endl;
        CloseHandle(tokenHandle);
        return "";
    }

    TOKEN_USER* tokenUser = reinterpret_cast<TOKEN_USER*>(tokenInfo.data());
    LPSTR sidString = nullptr;
    if (!ConvertSidToStringSidA(tokenUser->User.Sid, &sidString)) {
        std::cerr << "Failed to convert SID to string." << std::endl;
        CloseHandle(tokenHandle);
        return "";
    }

    std::string sid(sidString);
    LocalFree(sidString);
    CloseHandle(tokenHandle);
    return sid;
}

std::string CommandLib::GetCurrentUserName() {
    char userName[UNLEN + 1];
    DWORD userNameSize = sizeof(userName);
    if (!GetUserNameA(userName, &userNameSize)) {
        std::cerr << "Failed to get user name." << std::endl;
        return "";
    }
    return std::string(userName);
}