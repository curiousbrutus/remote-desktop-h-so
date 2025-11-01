@echo off
echo ========================================
echo   Remote Desktop Client - Starting...
echo ========================================
echo.

cd /d "%~dp0"

if "%1"=="" (
    echo Connecting to localhost:5900
    .\build\RemoteClient.exe 127.0.0.1 5900
) else (
    echo Connecting to %1:%2
    .\build\RemoteClient.exe %1 %2
)

pause
