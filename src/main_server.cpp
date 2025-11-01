#include "common/network.h"
#include "common/protocol.h"
#include "server/server.h"
#include <iostream>
#include <csignal>

using namespace RemoteDesktop;

static Server* g_server = nullptr;

void SignalHandler(int signal) {
    std::cout << "\nShutting down server..." << std::endl;
    if (g_server) {
        g_server->Stop();
    }
}

int main(int argc, char* argv[]) {
    std::cout << "==================================" << std::endl;
    std::cout << "  Remote Desktop Server" << std::endl;
    std::cout << "==================================" << std::endl;

    // Initialize Winsock
    if (!NetworkSocket::InitializeWinsock()) {
        std::cerr << "Failed to initialize Winsock" << std::endl;
        return 1;
    }

    // Parse port from command line (optional)
    uint16_t port = DEFAULT_PORT;
    if (argc > 1) {
        port = static_cast<uint16_t>(std::atoi(argv[1]));
    }

    // Create and start server
    Server server;
    g_server = &server;

    // Setup signal handler for graceful shutdown
    signal(SIGINT, SignalHandler);
    signal(SIGTERM, SignalHandler);

    if (!server.Start(port)) {
        std::cerr << "Failed to start server" << std::endl;
        NetworkSocket::CleanupWinsock();
        return 1;
    }

    std::cout << "\nPress Ctrl+C to stop the server...\n" << std::endl;

    // Wait for server to stop
    while (server.IsRunning()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Cleanup
    NetworkSocket::CleanupWinsock();
    
    std::cout << "Server terminated." << std::endl;
    return 0;
}
