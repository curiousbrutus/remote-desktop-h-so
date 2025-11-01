#pragma once
#include <windows.h>
#include <functional>
#include "common/protocol.h"

namespace RemoteDesktop {

class InputHandler {
public:
    using MouseCallback = std::function<void(const MouseEventData&)>;
    using KeyboardCallback = std::function<void(const KeyboardEventData&)>;

    InputHandler(HWND hwnd);
    ~InputHandler();

    // Enable/disable input capture
    void SetCaptureEnabled(bool enabled) { captureEnabled_ = enabled; }
    bool IsCaptureEnabled() const { return captureEnabled_; }

    // Set callbacks
    void SetMouseCallback(MouseCallback callback) { mouseCallback_ = callback; }
    void SetKeyboardCallback(KeyboardCallback callback) { keyboardCallback_ = callback; }

    // Handle Windows messages (call from WndProc)
    bool HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    HWND hwnd_;
    bool captureEnabled_;
    
    MouseCallback mouseCallback_;
    KeyboardCallback keyboardCallback_;
    
    // Track mouse button states
    bool leftButtonDown_;
    bool rightButtonDown_;
    bool middleButtonDown_;
};

} // namespace RemoteDesktop
