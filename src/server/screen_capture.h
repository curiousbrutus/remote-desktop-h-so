#pragma once
#include <windows.h>
#include <vector>
#include <cstdint>

namespace RemoteDesktop {

class ScreenCapture {
public:
    ScreenCapture();
    ~ScreenCapture();

    // Initialize screen capture for primary monitor
    bool Initialize();
    
    // Capture current screen to BGRA buffer
    bool CaptureScreen(std::vector<uint8_t>& outBgraData);
    
    // Get screen dimensions
    uint32_t GetWidth() const { return width_; }
    uint32_t GetHeight() const { return height_; }

private:
    HDC screenDC_;
    HDC memoryDC_;
    HBITMAP bitmap_;
    BITMAPINFO bitmapInfo_;
    
    uint32_t width_;
    uint32_t height_;
    bool initialized_;
};

} // namespace RemoteDesktop
