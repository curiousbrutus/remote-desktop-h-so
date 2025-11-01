#include "client/input_handler.h"
#include <windowsx.h>
#include <iostream>

namespace RemoteDesktop {

InputHandler::InputHandler(HWND hwnd)
    : hwnd_(hwnd)
    , captureEnabled_(false)
    , leftButtonDown_(false)
    , rightButtonDown_(false)
    , middleButtonDown_(false)
{
}

InputHandler::~InputHandler() {
}

bool InputHandler::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (!captureEnabled_) {
        return false; // Don't handle if not enabled
    }

    // Get mouse position
    int x = GET_X_LPARAM(lParam);
    int y = GET_Y_LPARAM(lParam);

    MouseEventData mouseEvent = {};
    KeyboardEventData keyEvent = {};
    bool handled = false;

    switch (uMsg) {
        case WM_MOUSEMOVE: {
            mouseEvent.eventType = MouseEventData::Type::MOVE;
            mouseEvent.x = x;
            mouseEvent.y = y;
            mouseEvent.wheelDelta = 0;
            if (mouseCallback_) {
                mouseCallback_(mouseEvent);
            }
            handled = true;
            break;
        }

        case WM_LBUTTONDOWN: {
            leftButtonDown_ = true;
            mouseEvent.eventType = MouseEventData::Type::LEFT_DOWN;
            mouseEvent.x = x;
            mouseEvent.y = y;
            mouseEvent.wheelDelta = 0;
            if (mouseCallback_) {
                mouseCallback_(mouseEvent);
            }
            handled = true;
            break;
        }

        case WM_LBUTTONUP: {
            leftButtonDown_ = false;
            mouseEvent.eventType = MouseEventData::Type::LEFT_UP;
            mouseEvent.x = x;
            mouseEvent.y = y;
            mouseEvent.wheelDelta = 0;
            if (mouseCallback_) {
                mouseCallback_(mouseEvent);
            }
            handled = true;
            break;
        }

        case WM_RBUTTONDOWN: {
            rightButtonDown_ = true;
            mouseEvent.eventType = MouseEventData::Type::RIGHT_DOWN;
            mouseEvent.x = x;
            mouseEvent.y = y;
            mouseEvent.wheelDelta = 0;
            if (mouseCallback_) {
                mouseCallback_(mouseEvent);
            }
            handled = true;
            break;
        }

        case WM_RBUTTONUP: {
            rightButtonDown_ = false;
            mouseEvent.eventType = MouseEventData::Type::RIGHT_UP;
            mouseEvent.x = x;
            mouseEvent.y = y;
            mouseEvent.wheelDelta = 0;
            if (mouseCallback_) {
                mouseCallback_(mouseEvent);
            }
            handled = true;
            break;
        }

        case WM_MBUTTONDOWN: {
            middleButtonDown_ = true;
            mouseEvent.eventType = MouseEventData::Type::MIDDLE_DOWN;
            mouseEvent.x = x;
            mouseEvent.y = y;
            mouseEvent.wheelDelta = 0;
            if (mouseCallback_) {
                mouseCallback_(mouseEvent);
            }
            handled = true;
            break;
        }

        case WM_MBUTTONUP: {
            middleButtonDown_ = false;
            mouseEvent.eventType = MouseEventData::Type::MIDDLE_UP;
            mouseEvent.x = x;
            mouseEvent.y = y;
            mouseEvent.wheelDelta = 0;
            if (mouseCallback_) {
                mouseCallback_(mouseEvent);
            }
            handled = true;
            break;
        }

        case WM_MOUSEWHEEL: {
            mouseEvent.eventType = MouseEventData::Type::WHEEL;
            mouseEvent.x = x;
            mouseEvent.y = y;
            mouseEvent.wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
            if (mouseCallback_) {
                mouseCallback_(mouseEvent);
            }
            handled = true;
            break;
        }

        case WM_KEYDOWN:
        case WM_SYSKEYDOWN: {
            keyEvent.eventType = KeyboardEventData::Type::KEY_DOWN;
            keyEvent.virtualKeyCode = static_cast<uint32_t>(wParam);
            keyEvent.scanCode = (lParam >> 16) & 0xFF;
            if (keyboardCallback_) {
                keyboardCallback_(keyEvent);
            }
            handled = true;
            break;
        }

        case WM_KEYUP:
        case WM_SYSKEYUP: {
            keyEvent.eventType = KeyboardEventData::Type::KEY_UP;
            keyEvent.virtualKeyCode = static_cast<uint32_t>(wParam);
            keyEvent.scanCode = (lParam >> 16) & 0xFF;
            if (keyboardCallback_) {
                keyboardCallback_(keyEvent);
            }
            handled = true;
            break;
        }
    }

    return handled;
}

} // namespace RemoteDesktop
