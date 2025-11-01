@echo off
echo ========================================
echo   Remote Desktop Server - Starting...
echo ========================================
echo.
echo Port: 5900 (default)
echo Press Ctrl+C to stop
echo.
echo Waiting for client connections...
echo.

cd /d "%~dp0"
.\build\RemoteServer.exe

pause
