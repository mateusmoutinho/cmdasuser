@ECHO OFF
REM CmdAsUser system . /c "C:\Windows\SysWOW64\cmd.exe /c C:\Windows\SysWOW64\martyn\test.cmd"
REM CmdAsUser system . /c "C:\Windows\SysWOW64\martyn\stopservice.exe "Sophos Endpoint Defense Service""
REM CmdAsUser system . /c "C:\Windows\SysWOW64\martyn\stopservice.exe "Sophos Endpoint Defense Service" stop"
REM CmdAsUser system . /c "C:\Windows\SysWOW64\martyn\stopservice.exe "XblAuthManager" enable"
REM CmdAsUser system . /c "C:\Windows\SysWOW64\martyn\stopservice.exe "Sophos Endpoint Defense Service" enable"
REM CmdAsUser system . /c "C:\Windows\SysWOW64\martyn\stopservice.exe "Sophos Endpoint Defense Service" stop"

REM sc queryex type=service state=all | find /i "SERVICE_NAME: Soph"
REM SERVICE_NAME: Sophos Endpoint Defense Service
REM SERVICE_NAME: Sophos File Scanner Service
REM SERVICE_NAME: Sophos Health Service
REM SERVICE_NAME: Sophos MCS Agent
REM SERVICE_NAME: Sophos MCS Client
REM SERVICE_NAME: Sophos System Protection Service

SET tmpPath=C:\Windows\System32\martyn
SET tmpLog=%tmpPath%\log.txt

ECHO Executing script: %~n0 >> %tmpLog%
ECHO Date: %DATE% >> %tmpLog%
ECHO Time: %TIME% >> %tmpLog%
ECHO User: %USERNAME% >> %tmpLog%

IF /I "%USERNAME%" NEQ "SYSTEM" (
    ECHO Script is not running as Local System >> %tmpLog%
    ECHO This script must be run as the Local System account.
    EXIT /B 1
)

sc stop "Sophos Endpoint Defense Service" >> %tmpLog%
IF %ERRORLEVEL% EQU 0 (
    ECHO Service stopped successfully >> %tmpLog%
) ELSE (
    ECHO Failed to stop service >> %tmpLog%
)


