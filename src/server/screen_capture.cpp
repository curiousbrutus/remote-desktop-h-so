#include "server/screen_capture.h"
#include <iostream>

namespace RemoteDesktop {

ScreenCapture::ScreenCapture()
    : screenDC_(nullptr)
    , memoryDC_(nullptr)
    , bitmap_(nullptr)
    , width_(0)
    , height_(0)
    , initialized_(false)
{
    ZeroMemory(&bitmapInfo_, sizeof(bitmapInfo_));
}

ScreenCapture::~ScreenCapture() {
    if (bitmap_) DeleteObject(bitmap_);
    if (memoryDC_) DeleteDC(memoryDC_);
    if (screenDC_) ReleaseDC(NULL, screenDC_);
}

bool ScreenCapture::Initialize() {
    // Get screen dimensions
    width_ = GetSystemMetrics(SM_CXSCREEN);
    height_ = GetSystemMetrics(SM_CYSCREEN);

    if (width_ == 0 || height_ == 0) {
        std::cerr << "Failed to get screen dimensions" << std::endl;
        return false;
    }

    // Get screen DC
    screenDC_ = GetDC(NULL);
    if (!screenDC_) {
        std::cerr << "Failed to get screen DC" << std::endl;
        return false;
    }

    // Create compatible DC for memory operations
    memoryDC_ = CreateCompatibleDC(screenDC_);
    if (!memoryDC_) {
        std::cerr << "Failed to create memory DC" << std::endl;
        return false;
    }

    // Create compatible bitmap
    bitmap_ = CreateCompatibleBitmap(screenDC_, width_, height_);
    if (!bitmap_) {
        std::cerr << "Failed to create bitmap" << std::endl;
        return false;
    }

    // Setup bitmap info for GetDIBits
    bitmapInfo_.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bitmapInfo_.bmiHeader.biWidth = width_;
    bitmapInfo_.bmiHeader.biHeight = -static_cast<LONG>(height_); // Top-down
    bitmapInfo_.bmiHeader.biPlanes = 1;
    bitmapInfo_.bmiHeader.biBitCount = 32; // BGRA
    bitmapInfo_.bmiHeader.biCompression = BI_RGB;
    bitmapInfo_.bmiHeader.biSizeImage = 0;

    initialized_ = true;
    std::cout << "Screen capture initialized: " << width_ << "x" << height_ << std::endl;
    return true;
}

bool ScreenCapture::CaptureScreen(std::vector<uint8_t>& outBgraData) {
    if (!initialized_) {
        std::cerr << "Screen capture not initialized" << std::endl;
        return false;
    }

    // Select bitmap into memory DC
    HGDIOBJ oldBitmap = SelectObject(memoryDC_, bitmap_);

    // Copy screen to bitmap
    if (!BitBlt(memoryDC_, 0, 0, width_, height_, 
                screenDC_, 0, 0, SRCCOPY)) {
        std::cerr << "BitBlt failed: " << GetLastError() << std::endl;
        SelectObject(memoryDC_, oldBitmap);
        return false;
    }

    // Allocate buffer
    size_t bufferSize = width_ * height_ * 4; // BGRA = 4 bytes per pixel
    outBgraData.resize(bufferSize);

    // Get bitmap bits
    int result = GetDIBits(memoryDC_, bitmap_, 0, height_,
                          outBgraData.data(), &bitmapInfo_, DIB_RGB_COLORS);

    SelectObject(memoryDC_, oldBitmap);

    if (result == 0) {
        std::cerr << "GetDIBits failed: " << GetLastError() << std::endl;
        return false;
    }

    return true;
}

} // namespace RemoteDesktop
