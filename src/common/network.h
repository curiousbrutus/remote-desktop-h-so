#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <vector>
#include <memory>
#include <cstdint>

namespace RemoteDesktop {

class NetworkSocket {
public:
    NetworkSocket();
    ~NetworkSocket();

    // Initialize Winsock (call once at program start)
    static bool InitializeWinsock();
    static void CleanupWinsock();

    // Server operations
    bool Listen(uint16_t port);
    std::unique_ptr<NetworkSocket> Accept();

    // Client operations
    bool Connect(const std::string& host, uint16_t port);

    // Data transfer
    bool Send(const void* data, size_t size);
    bool Receive(void* buffer, size_t size);
    bool ReceiveExact(void* buffer, size_t size); // Ensures exact size is received

    // Utilities
    bool IsValid() const { return socket_ != INVALID_SOCKET; }
    void Close();
    
    // Check if data is available (non-blocking)
    bool HasData(int timeoutMs = 0);
    
    std::string GetRemoteAddress() const;
    uint16_t GetRemotePort() const;

private:
    NetworkSocket(SOCKET socket); // Private constructor for accepted sockets
    
    SOCKET socket_;
    bool initialized_;
};

} // namespace RemoteDesktop
