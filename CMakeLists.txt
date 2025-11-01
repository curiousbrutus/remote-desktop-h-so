cmake_minimum_required(VERSION 3.15)
project(RemoteDesktop)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Windows specific settings
if(WIN32)
    add_definitions(-DUNICODE -D_UNICODE)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif()

# Common source files
set(COMMON_SOURCES
    src/common/network.cpp
    src/common/compression.cpp
)

# Server executable
add_executable(RemoteServer
    ${COMMON_SOURCES}
    src/server/screen_capture.cpp
    src/server/server.cpp
    src/main_server.cpp
)

# Client executable
add_executable(RemoteClient
    ${COMMON_SOURCES}
    src/client/client.cpp
    src/client/renderer.cpp
    src/main_client.cpp
)

# Link Windows libraries
if(WIN32)
    target_link_libraries(RemoteServer
        ws2_32      # Winsock
        gdi32       # GDI
        user32      # User input
        gdiplus     # GDI+
    )
    
    target_link_libraries(RemoteClient
        ws2_32      # Winsock
        gdi32       # GDI
        user32      # User input
        gdiplus     # GDI+
    )
endif()

# Include directories
target_include_directories(RemoteServer PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_include_directories(RemoteClient PRIVATE ${CMAKE_SOURCE_DIR}/src)

# Output directory
set_target_properties(RemoteServer RemoteClient PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build
)
