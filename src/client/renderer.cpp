#include "client/renderer.h"
#include <iostream>

namespace RemoteDesktop {

Renderer::Renderer()
    : hwnd_(nullptr)
    , memoryDC_(nullptr)
    , bitmap_(nullptr)
    , oldBitmap_(nullptr)
    , width_(0)
    , height_(0)
    , frameReady_(false)
{
}

Renderer::~Renderer() {
    if (oldBitmap_ && memoryDC_) {
        SelectObject(memoryDC_, oldBitmap_);
    }
    if (bitmap_) {
        DeleteObject(bitmap_);
    }
    if (memoryDC_) {
        DeleteDC(memoryDC_);
    }
}

bool Renderer::Initialize(HWND hwnd) {
    hwnd_ = hwnd;
    
    HDC windowDC = GetDC(hwnd_);
    if (!windowDC) {
        std::cerr << "Failed to get window DC" << std::endl;
        return false;
    }

    // Create memory DC
    memoryDC_ = CreateCompatibleDC(windowDC);
    ReleaseDC(hwnd_, windowDC);

    if (!memoryDC_) {
        std::cerr << "Failed to create memory DC" << std::endl;
        return false;
    }

    return true;
}

void Renderer::UpdateFrame(const std::vector<uint8_t>& bgraData, 
                           uint32_t width, uint32_t height) {
    std::lock_guard<std::mutex> lock(frameMutex_);

    // Check if dimensions changed
    if (width != width_ || height != height_) {
        // Delete old bitmap
        if (oldBitmap_ && memoryDC_) {
            SelectObject(memoryDC_, oldBitmap_);
            oldBitmap_ = nullptr;
        }
        if (bitmap_) {
            DeleteObject(bitmap_);
            bitmap_ = nullptr;
        }

        width_ = width;
        height_ = height;

        // Create new bitmap
        BITMAPINFO bmi = {};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = width_;
        bmi.bmiHeader.biHeight = -static_cast<LONG>(height_); // Top-down
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;

        void* bitsPtr = nullptr;
        HDC windowDC = GetDC(hwnd_);
        bitmap_ = CreateDIBSection(windowDC, &bmi, DIB_RGB_COLORS, 
                                   &bitsPtr, NULL, 0);
        ReleaseDC(hwnd_, windowDC);

        if (!bitmap_) {
            std::cerr << "Failed to create DIB section" << std::endl;
            return;
        }

        oldBitmap_ = (HBITMAP)SelectObject(memoryDC_, bitmap_);
    }

    // Copy frame data
    frameBuffer_ = bgraData;
    frameReady_ = true;

    // Copy to bitmap
    if (bitmap_) {
        BITMAPINFO bmi = {};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = width_;
        bmi.bmiHeader.biHeight = -static_cast<LONG>(height_);
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;

        SetDIBits(memoryDC_, bitmap_, 0, height_, 
                  frameBuffer_.data(), &bmi, DIB_RGB_COLORS);
    }

    // Trigger window redraw
    InvalidateRect(hwnd_, NULL, FALSE);
}

void Renderer::Render(HDC hdc) {
    std::lock_guard<std::mutex> lock(frameMutex_);

    if (!frameReady_ || !bitmap_) {
        // Fill with black if no frame
        RECT rect;
        GetClientRect(hwnd_, &rect);
        FillRect(hdc, &rect, (HBRUSH)GetStockObject(BLACK_BRUSH));
        return;
    }

    // Get client area size
    RECT clientRect;
    GetClientRect(hwnd_, &clientRect);
    int clientWidth = clientRect.right - clientRect.left;
    int clientHeight = clientRect.bottom - clientRect.top;

    // Blit from memory DC to window DC
    // Using StretchBlt to scale if window size differs from frame size
    StretchBlt(hdc, 
               0, 0, clientWidth, clientHeight,
               memoryDC_,
               0, 0, width_, height_,
               SRCCOPY);
}

} // namespace RemoteDesktop
