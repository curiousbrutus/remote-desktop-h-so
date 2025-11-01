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
    uint64_t frameCount = 0;

    while (connected_) {
        if (!ReceiveScreenFrame()) {
            std::cerr << "Failed to receive frame" << std::endl;
            connected_ = false;
            break;
        }

        frameCount++;
        if (frameCount % 100 == 0) {
            std::cout << "Received " << frameCount << " frames" << std::endl;
        }
    }

    std::cout << "Receive loop ended. Total frames: " << frameCount << std::endl;
}

bool Client::ReceiveScreenFrame() {
    // Receive packet header
    PacketHeader header;
    if (!socket_->ReceiveExact(&header, sizeof(header))) {
        return false;
    }

    // Verify packet type
    if (header.type != PacketType::SCREEN_FRAME) {
        std::cerr << "Unexpected packet type: " 
                  << static_cast<uint32_t>(header.type) << std::endl;
        return false;
    }

    // Check data size
    if (header.dataSize < sizeof(ScreenFrameData) || 
        header.dataSize > MAX_PACKET_SIZE) {
        std::cerr << "Invalid packet size: " << header.dataSize << std::endl;
        return false;
    }

    // Receive frame data
    ScreenFrameData frameData;
    if (!socket_->ReceiveExact(&frameData, sizeof(frameData))) {
        return false;
    }

    // Calculate JPEG data size
    uint32_t jpegSize = header.dataSize - sizeof(ScreenFrameData);
    
    // Receive JPEG data
    std::vector<uint8_t> jpegData(jpegSize);
    if (!socket_->ReceiveExact(jpegData.data(), jpegSize)) {
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
        std::cerr << "Failed to decompress JPEG" << std::endl;
        return false;
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

} // namespace RemoteDesktop
