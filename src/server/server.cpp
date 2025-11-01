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
    
    // Initialize input injector
    inputInjector_ = std::make_unique<InputInjector>();
    std::cout << "[SERVER] Input injector initialized" << std::endl;

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
    std::cout << "[SERVER] Starting frame transmission..." << std::endl;
    
    const auto frameDuration = std::chrono::milliseconds(1000 / SCREEN_FPS);
    auto lastFrameTime = std::chrono::steady_clock::now();

    uint64_t frameCount = 0;
    uint64_t failedFrames = 0;

    while (running_ && clientSocket->IsValid()) {
        // Check for incoming input events (non-blocking)
        if (!ReceiveAndProcessInput(clientSocket.get())) {
            std::cerr << "[SERVER] Failed to process input, client may have disconnected" << std::endl;
            break;
        }
        
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - lastFrameTime);

        if (elapsed >= frameDuration) {
            if (!SendScreenFrame(clientSocket.get())) {
                failedFrames++;
                std::cerr << "[SERVER] Failed to send screen frame (attempt " << failedFrames << ")" << std::endl;
                if (failedFrames > 5) {
                    std::cerr << "[SERVER] Too many failed frames, disconnecting client" << std::endl;
                    break;
                }
            } else {
                failedFrames = 0; // Reset on success
            }

            frameCount++;
            if (frameCount % 100 == 0) {
                std::cout << "[SERVER] Sent " << frameCount << " frames successfully" << std::endl;
            }

            lastFrameTime = now;
        } else {
            // Sleep to avoid busy waiting
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    std::cout << "[SERVER] Client disconnected. Total frames sent: " << frameCount 
              << ", Failed: " << failedFrames << std::endl;
}

bool Server::SendScreenFrame(NetworkSocket* socket) {
    // Capture screen
    std::vector<uint8_t> bgraData;
    if (!screenCapture_->CaptureScreen(bgraData)) {
        std::cerr << "[SERVER] Failed to capture screen" << std::endl;
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
        std::cerr << "[SERVER] Failed to compress frame" << std::endl;
        return false;
    }
    
    // Debug: Log compression ratio
    static int debugCount = 0;
    if (++debugCount % 100 == 0) {
        float ratio = (float)jpegData.size() / (float)bgraData.size() * 100.0f;
        std::cout << "[SERVER] Compression: " << bgraData.size() / 1024 << " KB -> " 
                  << jpegData.size() / 1024 << " KB (" << (int)ratio << "%)" << std::endl;
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
        std::cerr << "[SERVER] Failed to send packet header" << std::endl;
        return false;
    }

    // Send frame info
    if (!socket->Send(&frameData, sizeof(frameData))) {
        std::cerr << "[SERVER] Failed to send frame data" << std::endl;
        return false;
    }

    // Send JPEG data
    if (!socket->Send(jpegData.data(), jpegData.size())) {
        std::cerr << "[SERVER] Failed to send JPEG data (" << jpegData.size() << " bytes)" << std::endl;
        return false;
    }

    return true;
}

bool Server::ReceiveAndProcessInput(NetworkSocket* socket) {
    // Check if there's data available (non-blocking)
    if (!socket->HasData(1)) {
        return true; // No data, but not an error
    }

    // Receive packet header
    PacketHeader header;
    if (!socket->ReceiveExact(&header, sizeof(header))) {
        return false;
    }

    // Process based on packet type
    if (header.type == PacketType::MOUSE_INPUT) {
        if (header.dataSize != sizeof(MouseEventData)) {
            std::cerr << "[SERVER] Invalid mouse event size" << std::endl;
            return false;
        }

        MouseEventData event;
        if (!socket->ReceiveExact(&event, sizeof(event))) {
            return false;
        }

        if (!inputInjector_->InjectMouseEvent(event)) {
            std::cerr << "[SERVER] Failed to inject mouse event" << std::endl;
        }
    }
    else if (header.type == PacketType::KEYBOARD_INPUT) {
        if (header.dataSize != sizeof(KeyboardEventData)) {
            std::cerr << "[SERVER] Invalid keyboard event size" << std::endl;
            return false;
        }

        KeyboardEventData event;
        if (!socket->ReceiveExact(&event, sizeof(event))) {
            return false;
        }

        if (!inputInjector_->InjectKeyboardEvent(event)) {
            std::cerr << "[SERVER] Failed to inject keyboard event" << std::endl;
        }
    }
    else {
        std::cerr << "[SERVER] Unknown input packet type: " 
                  << static_cast<uint32_t>(header.type) << std::endl;
        return false;
    }

    return true;
}

} // namespace RemoteDesktop
