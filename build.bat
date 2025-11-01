@echo off
echo ========================================
echo   Remote Desktop - Manual Build Script
echo ========================================
echo.

REM Check for cl.exe (Visual Studio)
where cl.exe >nul 2>&1
if %errorlevel% equ 0 (
    echo Using Visual Studio compiler...
    goto :build_msvc
)

REM Check for g++ (MinGW)
where g++ >nul 2>&1
if %errorlevel% equ 0 (
    echo Using MinGW g++ compiler...
    goto :build_mingw
)

echo ERROR: No compiler found!
echo Please install one of:
echo   - Visual Studio 2019/2022 with C++ tools
echo   - MinGW-w64
echo   - CMake
pause
exit /b 1

:build_mingw
echo.
echo Building with MinGW...
echo.

REM Create output directory
if not exist "build" mkdir "build"

REM Compile common files
echo Compiling common/network.cpp...
g++ -c -std=c++17 -O2 -DUNICODE -D_UNICODE ^
    -I"src" ^
    src/common/network.cpp ^
    -o build/network.o

if %errorlevel% neq 0 exit /b 1

echo Compiling common/compression.cpp...
g++ -c -std=c++17 -O2 -DUNICODE -D_UNICODE ^
    -I"src" ^
    src/common/compression.cpp ^
    -o build/compression.o

if %errorlevel% neq 0 exit /b 1

REM Compile server files
echo Compiling server/screen_capture.cpp...
g++ -c -std=c++17 -O2 -DUNICODE -D_UNICODE ^
    -I"src" ^
    src/server/screen_capture.cpp ^
    -o build/screen_capture.o

if %errorlevel% neq 0 exit /b 1

echo Compiling server/server.cpp...
g++ -c -std=c++17 -O2 -DUNICODE -D_UNICODE ^
    -I"src" ^
    src/server/server.cpp ^
    -o build/server.o

if %errorlevel% neq 0 exit /b 1

echo Compiling main_server.cpp...
g++ -c -std=c++17 -O2 -DUNICODE -D_UNICODE ^
    -I"src" ^
    src/main_server.cpp ^
    -o build/main_server.o

if %errorlevel% neq 0 exit /b 1

REM Link server executable
echo Linking RemoteServer.exe...
g++ -std=c++17 -O2 ^
    build/network.o ^
    build/compression.o ^
    build/screen_capture.o ^
    build/server.o ^
    build/main_server.o ^
    -o build/RemoteServer.exe ^
    -lws2_32 -lgdi32 -luser32 -lgdiplus -lole32 -static

if %errorlevel% neq 0 exit /b 1

REM Compile client files
echo Compiling client/client.cpp...
g++ -c -std=c++17 -O2 -DUNICODE -D_UNICODE ^
    -I"src" ^
    src/client/client.cpp ^
    -o build/client.o

if %errorlevel% neq 0 exit /b 1

echo Compiling client/renderer.cpp...
g++ -c -std=c++17 -O2 -DUNICODE -D_UNICODE ^
    -I"src" ^
    src/client/renderer.cpp ^
    -o build/renderer.o

if %errorlevel% neq 0 exit /b 1

echo Compiling main_client.cpp...
g++ -c -std=c++17 -O2 -DUNICODE -D_UNICODE ^
    -I"src" ^
    src/main_client.cpp ^
    -o build/main_client.o

if %errorlevel% neq 0 exit /b 1

REM Link client executable
echo Linking RemoteClient.exe...
g++ -std=c++17 -O2 ^
    build/network.o ^
    build/compression.o ^
    build/client.o ^
    build/renderer.o ^
    build/main_client.o ^
    -o build/RemoteClient.exe ^
    -lws2_32 -lgdi32 -luser32 -lgdiplus -lole32 -static

if %errorlevel% neq 0 exit /b 1

echo.
echo ========================================
echo   BUILD SUCCESSFUL!
echo ========================================
echo Server: build\RemoteServer.exe
echo Client: build\RemoteClient.exe
echo.
echo Kullanim:
echo   Server: .\build\RemoteServer.exe
echo   Client: .\build\RemoteClient.exe 127.0.0.1 5900
echo.
pause
exit /b 0

:build_msvc
echo.
echo Please run this from a Visual Studio Developer Command Prompt
echo Or use "Developer PowerShell for VS 2022"
echo.
pause
exit /b 1
