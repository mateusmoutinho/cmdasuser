#define _WIN32_WINNT 0x0A00

#include <windows.h>
#include <winsvc.h>
#include <vector>
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


bool IsElevated() {
    BOOL isElevated = FALSE;
    HANDLE token = NULL;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) {
        TOKEN_ELEVATION elevation;
        DWORD size;
        if (GetTokenInformation(token, TokenElevation, &elevation, sizeof(elevation), &size)) {
            isElevated = elevation.TokenIsElevated;
        }
        CloseHandle(token);
    }
    return isElevated;
}

void ElevateSelf(ILogger& logger) {
    wchar_t exePath[MAX_PATH];
    GetModuleFileName(NULL, exePath, MAX_PATH);

    SHELLEXECUTEINFO sei = { sizeof(sei) };
    sei.lpVerb = L"runas";
    sei.lpFile = exePath;
    sei.hwnd = NULL;
    sei.nShow = SW_NORMAL;

    if (!ShellExecuteEx(&sei)) {
        DWORD error = GetLastError();
        if (error == ERROR_CANCELLED) {
            logger.Log(L"User declined the elevation.");
        }
        else {
            logger.Log(L"Failed to elevate: " + to_wstring(error));
        }
    }
}

bool IsProtectedService(SC_HANDLE hService, ILogger& logger) {
    DWORD bytesNeeded = 0;
    if (!QueryServiceConfig2(hService, SERVICE_CONFIG_LAUNCH_PROTECTED, NULL, 0, &bytesNeeded) && GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
        logger.Log(L"QueryServiceConfig2 failed: " + to_wstring(GetLastError()));
        return false;
    }

    std::vector<BYTE> buffer(bytesNeeded);
    if (!QueryServiceConfig2(hService, SERVICE_CONFIG_LAUNCH_PROTECTED, buffer.data(), bytesNeeded, &bytesNeeded)) {
        logger.Log(L"QueryServiceConfig2 failed: " + to_wstring(GetLastError()));
        return false;
    }

    SERVICE_LAUNCH_PROTECTED_INFO* protectionInfo = reinterpret_cast<SERVICE_LAUNCH_PROTECTED_INFO*>(buffer.data());
    return protectionInfo->dwLaunchProtected != SERVICE_LAUNCH_PROTECTED_NONE;
}

void LogDependentServices(SC_HANDLE hService, ILogger& logger) {
    DWORD bytesNeeded = 0;
    DWORD serviceCount = 0;
    if (!EnumDependentServices(hService, SERVICE_ACTIVE, NULL, 0, &bytesNeeded, &serviceCount) && GetLastError() != ERROR_MORE_DATA) {
        logger.Log(L"EnumDependentServices failed: " + to_wstring(GetLastError()));
        return;
    }

    std::vector<BYTE> buffer(bytesNeeded);
    LPENUM_SERVICE_STATUS dependencies = reinterpret_cast<LPENUM_SERVICE_STATUS>(buffer.data());
    if (!EnumDependentServices(hService, SERVICE_ACTIVE, dependencies, bytesNeeded, &bytesNeeded, &serviceCount)) {
        logger.Log(L"EnumDependentServices failed: " + to_wstring(GetLastError()));
        return;
    }

    logger.Log(L"Dependent services to stop:");
    for (DWORD i = 0; i < serviceCount; ++i) {
        logger.Log(L"  " + std::wstring(dependencies[i].lpServiceName));
    }
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

    if (IsProtectedService(hService, logger)) {
        logger.Log(L"Service is a protected service.");
    }
    else {
        logger.Log(L"Service is not a protected service.");
    }

    LogDependentServices(hService, logger);

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

void SetServiceStartupType(const std::wstring& serviceName, DWORD startupType, ILogger& logger) {

    logger.Log(L"Attempting to change startup type for service: " + serviceName);

    SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (hSCManager == NULL) {
        logger.Log(L"OpenSCManager failed: " + to_wstring(GetLastError()));
        return;
    }

    SC_HANDLE hService = OpenService(hSCManager, serviceName.c_str(), SERVICE_CHANGE_CONFIG);
    if (hService == NULL) {
        logger.Log(L"OpenService failed: " + to_wstring(GetLastError()));
        CloseServiceHandle(hSCManager);
        return;
    }

    if (!ChangeServiceConfig(hService, SERVICE_NO_CHANGE, startupType, SERVICE_NO_CHANGE, NULL, NULL, NULL, NULL, NULL, NULL, NULL)) {
        logger.Log(L"ChangeServiceConfig failed: " + to_wstring(GetLastError()));
    }
    else {
        logger.Log(L"Service startup type changed successfully.");
    }

    CloseServiceHandle(hService);
    CloseServiceHandle(hSCManager);
}

int wmain(int argc, wchar_t* argv[]) {

	OutputDebugString(L"StopService STARTING\n");

    if (argc != 3) {
        std::wcerr << L"Usage: " << argv[0] << L" <ServiceName> <start|stop|enable|disable>" << std::endl;
        return 1;
    }

    std::wstring action = argv[2];

    wchar_t exePath[MAX_PATH];
    GetModuleFileName(NULL, exePath, MAX_PATH);
    PathRemoveFileSpec(exePath);
    std::wstring logFilePath = std::wstring(exePath) + L"\\log.txt";

    DebugLogger logger;
    //FileLogger logger(logFilePath);

    try {
        if (!IsElevated()) {
            ElevateSelf(logger);
        }

        logger.Log(L"IsElevated:: " + std::to_wstring(IsElevated() ? 1 : 0));

        if (action == L"stop") {
            StopService(argv[1], logger);
        }
        else if (action == L"enable") {
            SetServiceStartupType(argv[1], SERVICE_AUTO_START, logger);
        }
        else if (action == L"disable") {
            SetServiceStartupType(argv[1], SERVICE_DISABLED, logger);
        }
        else {
            std::wcerr << L"Invalid action: " << action << std::endl;
            return 1;
        }
    }
    catch (const std::exception& e) {
        std::wcerr << e.what() << std::endl;
        return 1;
    }

    OutputDebugString(L"StopService ENDING\n");

    return 0;
}

