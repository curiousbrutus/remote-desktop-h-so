#include "client/client.h"
#include "common/protocol.h"
#include "common/compression.h"
#include <iostream>

namespace RemoteDesktop {

Client::Client()
    : connected_(false)
{
}

Client::~Client() {
    Disconnect();
}

bool Client::Connect(const std::string& host, uint16_t port) {
    if (connected_) {
        std::cerr << "Already connected" << std::endl;
        return false;
    }

    // Create socket and connect
    socket_ = std::make_unique<NetworkSocket>();
    if (!socket_->Connect(host, port)) {
        std::cerr << "Failed to connect to " << host << ":" << port << std::endl;
        return false;
    }

    connected_ = true;
    receiveThread_ = std::thread(&Client::ReceiveLoop, this);

    std::cout << "Connected to server: " << host << ":" << port << std::endl;
    return true;
}

void Client::Disconnect() {
    if (!connected_) return;

    connected_ = false;

    if (socket_) {
        socket_->Close();
    }

    if (receiveThread_.joinable()) {
        receiveThread_.join();
    }

    std::cout << "Disconnected from server" << std::endl;
}

void Client::ReceiveLoop() {
    std::cout << "[CLIENT] Starting receive loop..." << std::endl;
    uint64_t frameCount = 0;
    uint64_t errorCount = 0;

    while (connected_) {
        if (!ReceiveScreenFrame()) {
            errorCount++;
            std::cerr << "[CLIENT] Failed to receive frame (error " << errorCount << ")" << std::endl;
            
            // Don't disconnect immediately, might be temporary network issue
            if (errorCount > 3) {
                std::cerr << "[CLIENT] Too many consecutive errors, disconnecting" << std::endl;
                connected_ = false;
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }
        
        errorCount = 0; // Reset on success
        frameCount++;
        
        if (frameCount == 1) {
            std::cout << "[CLIENT] First frame received successfully!" << std::endl;
        }
        
        if (frameCount % 100 == 0) {
            std::cout << "[CLIENT] Received " << frameCount << " frames" << std::endl;
        }
    }

    std::cout << "[CLIENT] Receive loop ended. Total frames: " << frameCount << std::endl;
}

bool Client::ReceiveScreenFrame() {
    // Receive packet header
    PacketHeader header;
    if (!socket_->ReceiveExact(&header, sizeof(header))) {
        std::cerr << "[CLIENT] Failed to receive packet header" << std::endl;
        return false;
    }

    // Verify packet type
    if (header.type != PacketType::SCREEN_FRAME) {
        std::cerr << "[CLIENT] Unexpected packet type: " 
                  << static_cast<uint32_t>(header.type) << std::endl;
        return false;
    }

    // Check data size
    if (header.dataSize < sizeof(ScreenFrameData) || 
        header.dataSize > MAX_PACKET_SIZE) {
        std::cerr << "[CLIENT] Invalid packet size: " << header.dataSize << std::endl;
        return false;
    }
    
    // Debug: First frame info
    static bool firstFrame = true;
    if (firstFrame) {
        std::cout << "[CLIENT] Receiving first frame, data size: " << header.dataSize << " bytes" << std::endl;
        firstFrame = false;
    }

    // Receive frame data
    ScreenFrameData frameData;
    if (!socket_->ReceiveExact(&frameData, sizeof(frameData))) {
        std::cerr << "[CLIENT] Failed to receive frame data structure" << std::endl;
        return false;
    }

    // Calculate JPEG data size
    uint32_t jpegSize = header.dataSize - sizeof(ScreenFrameData);
    
    // Debug: First frame dimensions
    static bool firstFrameData = true;
    if (firstFrameData) {
        std::cout << "[CLIENT] Frame dimensions: " << frameData.width << "x" << frameData.height 
                  << ", JPEG size: " << jpegSize << " bytes" << std::endl;
        firstFrameData = false;
    }
    
    // Receive JPEG data
    std::vector<uint8_t> jpegData(jpegSize);
    if (!socket_->ReceiveExact(jpegData.data(), jpegSize)) {
        std::cerr << "[CLIENT] Failed to receive JPEG data (" << jpegSize << " bytes)" << std::endl;
        return false;
    }

    // Decompress JPEG
    std::vector<uint8_t> bgraData;
    uint32_t width, height;
    
    if (!Compression::DecompressFromJPEG(
            jpegData.data(), 
            jpegSize, 
            bgraData, 
            width, 
            height)) {
        std::cerr << "[CLIENT] Failed to decompress JPEG (size: " << jpegSize << " bytes)" << std::endl;
        return false;
    }
    
    // Debug: First successful frame
    static bool firstSuccess = true;
    if (firstSuccess) {
        std::cout << "[CLIENT] Successfully decoded first frame: " << width << "x" << height << std::endl;
        firstSuccess = false;
    }

    // Verify dimensions match
    if (width != frameData.width || height != frameData.height) {
        std::cerr << "Dimension mismatch: expected " 
                  << frameData.width << "x" << frameData.height
                  << ", got " << width << "x" << height << std::endl;
        return false;
    }

    // Call frame callback if set
    if (frameCallback_) {
        frameCallback_(bgraData, width, height);
    }

    return true;
}

bool Client::SendMouseEvent(const MouseEventData& event) {
    if (!connected_) return false;

    PacketHeader header;
    header.type = PacketType::MOUSE_INPUT;
    header.dataSize = sizeof(MouseEventData);

    if (!socket_->Send(&header, sizeof(header))) {
        return false;
    }

    if (!socket_->Send(&event, sizeof(event))) {
        return false;
    }

    return true;
}

bool Client::SendKeyboardEvent(const KeyboardEventData& event) {
    if (!connected_) return false;

    PacketHeader header;
    header.type = PacketType::KEYBOARD_INPUT;
    header.dataSize = sizeof(KeyboardEventData);

    if (!socket_->Send(&header, sizeof(header))) {
        return false;
    }

    if (!socket_->Send(&event, sizeof(event))) {
        return false;
    }

    return true;
}

} // namespace RemoteDesktop
