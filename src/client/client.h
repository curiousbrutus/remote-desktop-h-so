#pragma once
#include "common/network.h"
#include "common/protocol.h"
#include <memory>
#include <atomic>
#include <thread>
#include <functional>
#include <vector>

namespace RemoteDesktop {

class Client {
public:
    using FrameCallback = std::function<void(
        const std::vector<uint8_t>& bgraData, 
        uint32_t width, 
        uint32_t height)>;

    Client();
    ~Client();

    // Connect to server
    bool Connect(const std::string& host, uint16_t port);
    
    // Disconnect from server
    void Disconnect();
    
    // Check if connected
    bool IsConnected() const { return connected_; }

    // Set callback for received frames
    void SetFrameCallback(FrameCallback callback) { 
        frameCallback_ = callback; 
    }
    
    // Send input events to server
    bool SendMouseEvent(const MouseEventData& event);
    bool SendKeyboardEvent(const KeyboardEventData& event);

private:
    void ReceiveLoop();
    bool ReceiveScreenFrame();

    std::unique_ptr<NetworkSocket> socket_;
    std::atomic<bool> connected_;
    std::thread receiveThread_;
    
    FrameCallback frameCallback_;
};

} // namespace RemoteDesktop
