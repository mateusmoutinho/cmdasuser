#include <windows.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <sddl.h>
#include <shlwapi.h>

#pragma comment(lib, "Shlwapi.lib")

class ILogger {
public:
    virtual void Log(const std::wstring& message) = 0;
    virtual ~ILogger() = default;
};

class FileLogger : public ILogger {
public:
    FileLogger(const std::wstring& filePath) {
        logFile.open(filePath, std::ios::out | std::ios::app);
        if (!logFile.is_open()) {
            throw std::runtime_error("Failed to open log file.");
        }
    }

    void Log(const std::wstring& message) override {
        logFile << message << std::endl;
    }

    ~FileLogger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }

private:
    std::wofstream logFile;
};

class DebugLogger : public ILogger {
public:
    void Log(const std::wstring& message) override {
        OutputDebugString(message.c_str());
    }
};

std::wstring to_wstring(int value) {
    std::wstringstream wss;
    wss << value;
    return wss.str();
}

bool IsLocalSystemAccount() {

    HANDLE token = NULL;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) {
        return false;
    }

    DWORD size = 0;
    GetTokenInformation(token, TokenUser, NULL, 0, &size);
    if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
        CloseHandle(token);
        return false;
    }

    TOKEN_USER* tokenUser = (TOKEN_USER*)malloc(size);
    if (!GetTokenInformation(token, TokenUser, tokenUser, size, &size)) {
        free(tokenUser);
        CloseHandle(token);
        return false;
    }

    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
    PSID localSystemSid = NULL;
    if (!AllocateAndInitializeSid(&ntAuthority, 1, SECURITY_LOCAL_SYSTEM_RID, 0, 0, 0, 0, 0, 0, 0, &localSystemSid)) {
        free(tokenUser);
        CloseHandle(token);
        return false;
    }

    BOOL isLocalSystem = EqualSid(tokenUser->User.Sid, localSystemSid);

    FreeSid(localSystemSid);
    free(tokenUser);
    CloseHandle(token);

    return isLocalSystem == TRUE;
}

void StopService(const std::wstring& serviceName, ILogger& logger) {

    //OutputDebugString(L"StopService110\n");
    logger.Log(L"Attempting to stop service: " + serviceName);

    if (IsLocalSystemAccount()) {
        logger.Log(L"Running as Local System account.");
    }
    else {
        logger.Log(L"Not running as Local System account.");
    }

    SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (hSCManager == NULL) {
        logger.Log(L"OpenSCManager failed: " + std::to_wstring(GetLastError()));
        return;
    }

    SC_HANDLE hService = OpenService(hSCManager, serviceName.c_str(), SERVICE_STOP | SERVICE_QUERY_STATUS);
    if (hService == NULL) {
        logger.Log(L"OpenService failed: " + std::to_wstring(GetLastError()));
        CloseServiceHandle(hSCManager);
        return;
    }

    SERVICE_STATUS_PROCESS ssp;
    DWORD bytesNeeded;
    if (!QueryServiceStatusEx(hService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp, sizeof(SERVICE_STATUS_PROCESS), &bytesNeeded)) {
        logger.Log(L"QueryServiceStatusEx failed: " + std::to_wstring(GetLastError()));
        CloseServiceHandle(hService);
        CloseServiceHandle(hSCManager);
        return;
    }

    if (ssp.dwCurrentState == SERVICE_STOPPED) {
        logger.Log(L"Service is already stopped.");
    }
    else {
        SERVICE_STATUS ss;
        if (!ControlService(hService, SERVICE_CONTROL_STOP, &ss)) {
            logger.Log(L"ControlService failed: " + std::to_wstring(GetLastError()));
        }
        else {
            logger.Log(L"Service stop pending...");
            Sleep(1000);

            while (QueryServiceStatusEx(hService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp, sizeof(SERVICE_STATUS_PROCESS), &bytesNeeded)) {
                if (ssp.dwCurrentState == SERVICE_STOPPED) {
                    logger.Log(L"Service stopped successfully.");
                    break;
                }
                Sleep(1000);
            }
        }
    }

    CloseServiceHandle(hService);
    CloseServiceHandle(hSCManager);
}

int wmain(int argc, wchar_t* argv[]) {

    /*if (argc != 2) {
        std::wcerr << L"Usage: " << argv[0] << L" <ServiceName>" << std::endl;
        return 1;
    }*/

	OutputDebugString(L"StopService STARTING\n");

    wchar_t exePath[MAX_PATH];
    GetModuleFileName(NULL, exePath, MAX_PATH);
    PathRemoveFileSpec(exePath);
    std::wstring logFilePath = std::wstring(exePath) + L"\\log.txt";

    try {
        DebugLogger logger;
        //FileLogger logger(logFilePath);
        StopService(L"Sophos Endpoint Defense Service", logger);
    }
    catch (const std::exception& e) {

        OutputDebugString(L"StopService ERROR\n");
        std::wcerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
