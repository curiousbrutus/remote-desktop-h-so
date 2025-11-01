#pragma once
#include "common/network.h"
#include "server/screen_capture.h"
#include "server/input_injector.h"
#include <memory>
#include <atomic>
#include <thread>

namespace RemoteDesktop {

class Server {
public:
    Server();
    ~Server();

    // Start server and listen for connections
    bool Start(uint16_t port);
    
    // Stop server
    void Stop();
    
    // Check if server is running
    bool IsRunning() const { return running_; }

private:
    void ServerLoop();
    void HandleClient(std::unique_ptr<NetworkSocket> clientSocket);
    
    bool SendScreenFrame(NetworkSocket* socket);
    bool ReceiveAndProcessInput(NetworkSocket* socket);

    std::unique_ptr<NetworkSocket> serverSocket_;
    std::unique_ptr<ScreenCapture> screenCapture_;
    std::unique_ptr<InputInjector> inputInjector_;
    
    std::atomic<bool> running_;
    std::thread serverThread_;
    
    uint16_t port_;
};

} // namespace RemoteDesktop
