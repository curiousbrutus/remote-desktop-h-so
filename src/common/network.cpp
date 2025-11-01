#include "common/network.h"
#include <iostream>

#pragma comment(lib, "ws2_32.lib")

namespace RemoteDesktop {

static int wsaRefCount = 0;

NetworkSocket::NetworkSocket() 
    : socket_(INVALID_SOCKET), initialized_(false) {
}

NetworkSocket::NetworkSocket(SOCKET socket) 
    : socket_(socket), initialized_(true) {
}

NetworkSocket::~NetworkSocket() {
    Close();
}

bool NetworkSocket::InitializeWinsock() {
    if (wsaRefCount++ > 0) {
        return true; // Already initialized
    }

    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << std::endl;
        wsaRefCount--;
        return false;
    }
    return true;
}

void NetworkSocket::CleanupWinsock() {
    if (--wsaRefCount <= 0) {
        WSACleanup();
        wsaRefCount = 0;
    }
}

bool NetworkSocket::Listen(uint16_t port) {
    socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_ == INVALID_SOCKET) {
        std::cerr << "Socket creation failed: " << WSAGetLastError() << std::endl;
        return false;
    }

    // Allow address reuse
    int opt = 1;
    setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

    sockaddr_in serverAddr = {};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(socket_, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed: " << WSAGetLastError() << std::endl;
        Close();
        return false;
    }

    if (listen(socket_, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed: " << WSAGetLastError() << std::endl;
        Close();
        return false;
    }

    initialized_ = true;
    return true;
}

std::unique_ptr<NetworkSocket> NetworkSocket::Accept() {
    sockaddr_in clientAddr = {};
    int clientAddrSize = sizeof(clientAddr);
    
    SOCKET clientSocket = accept(socket_, (sockaddr*)&clientAddr, &clientAddrSize);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Accept failed: " << WSAGetLastError() << std::endl;
        return nullptr;
    }

    return std::unique_ptr<NetworkSocket>(new NetworkSocket(clientSocket));
}

bool NetworkSocket::Connect(const std::string& host, uint16_t port) {
    socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_ == INVALID_SOCKET) {
        std::cerr << "Socket creation failed: " << WSAGetLastError() << std::endl;
        return false;
    }

    sockaddr_in serverAddr = {};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);

    // Convert IP address
    if (inet_pton(AF_INET, host.c_str(), &serverAddr.sin_addr) <= 0) {
        std::cerr << "Invalid address: " << host << std::endl;
        Close();
        return false;
    }

    if (connect(socket_, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Connection failed: " << WSAGetLastError() << std::endl;
        Close();
        return false;
    }

    initialized_ = true;
    return true;
}

bool NetworkSocket::Send(const void* data, size_t size) {
    if (!IsValid()) return false;

    const char* ptr = static_cast<const char*>(data);
    size_t totalSent = 0;

    while (totalSent < size) {
        int sent = send(socket_, ptr + totalSent, static_cast<int>(size - totalSent), 0);
        if (sent == SOCKET_ERROR) {
            std::cerr << "Send failed: " << WSAGetLastError() << std::endl;
            return false;
        }
        totalSent += sent;
    }

    return true;
}

bool NetworkSocket::Receive(void* buffer, size_t size) {
    if (!IsValid()) return false;

    int received = recv(socket_, static_cast<char*>(buffer), static_cast<int>(size), 0);
    if (received == SOCKET_ERROR || received == 0) {
        return false;
    }

    return true;
}

bool NetworkSocket::ReceiveExact(void* buffer, size_t size) {
    if (!IsValid()) return false;

    char* ptr = static_cast<char*>(buffer);
    size_t totalReceived = 0;

    while (totalReceived < size) {
        int received = recv(socket_, ptr + totalReceived, 
                          static_cast<int>(size - totalReceived), 0);
        if (received == SOCKET_ERROR || received == 0) {
            std::cerr << "ReceiveExact failed: " << WSAGetLastError() << std::endl;
            return false;
        }
        totalReceived += received;
    }

    return true;
}

void NetworkSocket::Close() {
    if (socket_ != INVALID_SOCKET) {
        closesocket(socket_);
        socket_ = INVALID_SOCKET;
    }
    initialized_ = false;
}

std::string NetworkSocket::GetRemoteAddress() const {
    sockaddr_in addr = {};
    int addrLen = sizeof(addr);
    
    if (getpeername(socket_, (sockaddr*)&addr, &addrLen) == SOCKET_ERROR) {
        return "";
    }

    char ipStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr.sin_addr, ipStr, INET_ADDRSTRLEN);
    return std::string(ipStr);
}

uint16_t NetworkSocket::GetRemotePort() const {
    sockaddr_in addr = {};
    int addrLen = sizeof(addr);
    
    if (getpeername(socket_, (sockaddr*)&addr, &addrLen) == SOCKET_ERROR) {
        return 0;
    }

    return ntohs(addr.sin_port);
}

} // namespace RemoteDesktop
