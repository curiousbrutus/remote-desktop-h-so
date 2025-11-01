@echo off
echo ========================================
echo   Remote Desktop - Full Test
echo ========================================
echo.
echo This will test the connection on localhost
echo.
echo Starting server in background...
start "Remote Desktop Server" cmd /c "cd /d "%~dp0" && .\build\RemoteServer.exe"
echo.
echo Waiting 2 seconds for server to start...
timeout /t 2 /nobreak >nul
echo.
echo Starting client...
echo.
.\build\RemoteClient.exe 127.0.0.1 5900
pause
