#include "server/server.h"
#include "common/protocol.h"
#include "common/compression.h"
#include <iostream>
#include <chrono>

namespace RemoteDesktop {

Server::Server()
    : running_(false)
    , port_(DEFAULT_PORT)
{
}

Server::~Server() {
    Stop();
}

bool Server::Start(uint16_t port) {
    if (running_) {
        std::cerr << "Server is already running" << std::endl;
        return false;
    }

    port_ = port;

    // Initialize screen capture
    screenCapture_ = std::make_unique<ScreenCapture>();
    if (!screenCapture_->Initialize()) {
        std::cerr << "Failed to initialize screen capture" << std::endl;
        return false;
    }

    // Create server socket
    serverSocket_ = std::make_unique<NetworkSocket>();
    if (!serverSocket_->Listen(port_)) {
        std::cerr << "Failed to start server on port " << port_ << std::endl;
        return false;
    }

    running_ = true;
    serverThread_ = std::thread(&Server::ServerLoop, this);

    std::cout << "Server started on port " << port_ << std::endl;
    std::cout << "Screen size: " << screenCapture_->GetWidth() 
              << "x" << screenCapture_->GetHeight() << std::endl;
    std::cout << "Waiting for connections..." << std::endl;

    return true;
}

void Server::Stop() {
    if (!running_) return;

    running_ = false;
    
    if (serverSocket_) {
        serverSocket_->Close();
    }

    if (serverThread_.joinable()) {
        serverThread_.join();
    }

    std::cout << "Server stopped" << std::endl;
}

void Server::ServerLoop() {
    while (running_) {
        // Accept incoming connection
        auto clientSocket = serverSocket_->Accept();
        if (!clientSocket) {
            if (running_) {
                std::cerr << "Failed to accept client connection" << std::endl;
            }
            break;
        }

        std::cout << "Client connected from " 
                  << clientSocket->GetRemoteAddress() << ":" 
                  << clientSocket->GetRemotePort() << std::endl;

        // Handle client in current thread (single client for now)
        HandleClient(std::move(clientSocket));
    }
}

void Server::HandleClient(std::unique_ptr<NetworkSocket> clientSocket) {
    const auto frameDuration = std::chrono::milliseconds(1000 / SCREEN_FPS);
    auto lastFrameTime = std::chrono::steady_clock::now();

    uint64_t frameCount = 0;

    while (running_ && clientSocket->IsValid()) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - lastFrameTime);

        if (elapsed >= frameDuration) {
            if (!SendScreenFrame(clientSocket.get())) {
                std::cerr << "Failed to send screen frame" << std::endl;
                break;
            }

            frameCount++;
            if (frameCount % 100 == 0) {
                std::cout << "Sent " << frameCount << " frames" << std::endl;
            }

            lastFrameTime = now;
        } else {
            // Sleep to avoid busy waiting
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    std::cout << "Client disconnected. Total frames sent: " << frameCount << std::endl;
}

bool Server::SendScreenFrame(NetworkSocket* socket) {
    // Capture screen
    std::vector<uint8_t> bgraData;
    if (!screenCapture_->CaptureScreen(bgraData)) {
        return false;
    }

    // Compress to JPEG
    std::vector<uint8_t> jpegData;
    if (!Compression::CompressToJPEG(
            bgraData.data(),
            screenCapture_->GetWidth(),
            screenCapture_->GetHeight(),
            JPEG_QUALITY,
            jpegData)) {
        std::cerr << "Failed to compress frame" << std::endl;
        return false;
    }

    // Prepare packet header
    PacketHeader header;
    header.type = PacketType::SCREEN_FRAME;
    header.dataSize = sizeof(ScreenFrameData) + static_cast<uint32_t>(jpegData.size());

    // Prepare frame data
    ScreenFrameData frameData;
    frameData.width = screenCapture_->GetWidth();
    frameData.height = screenCapture_->GetHeight();
    frameData.compressionQuality = JPEG_QUALITY;

    // Send header
    if (!socket->Send(&header, sizeof(header))) {
        return false;
    }

    // Send frame info
    if (!socket->Send(&frameData, sizeof(frameData))) {
        return false;
    }

    // Send JPEG data
    if (!socket->Send(jpegData.data(), jpegData.size())) {
        return false;
    }

    return true;
}

} // namespace RemoteDesktop
