#pragma once
#include <windows.h>
#include <vector>
#include <mutex>
#include <cstdint>

namespace RemoteDesktop {

class Renderer {
public:
    Renderer();
    ~Renderer();

    // Initialize renderer with window handle
    bool Initialize(HWND hwnd);
    
    // Update frame to display
    void UpdateFrame(const std::vector<uint8_t>& bgraData, 
                     uint32_t width, uint32_t height);
    
    // Render current frame to window
    void Render(HDC hdc);
    
    // Get current frame dimensions
    uint32_t GetWidth() const { return width_; }
    uint32_t GetHeight() const { return height_; }

private:
    HWND hwnd_;
    HDC memoryDC_;
    HBITMAP bitmap_;
    HBITMAP oldBitmap_;
    
    std::vector<uint8_t> frameBuffer_;
    uint32_t width_;
    uint32_t height_;
    
    std::mutex frameMutex_;
    bool frameReady_;
};

} // namespace RemoteDesktop
