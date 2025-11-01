#include "server/input_injector.h"
#include <windows.h>
#include <iostream>

namespace RemoteDesktop {

InputInjector::InputInjector() {
    screenWidth_ = GetSystemMetrics(SM_CXSCREEN);
    screenHeight_ = GetSystemMetrics(SM_CYSCREEN);
}

InputInjector::~InputInjector() {
}

bool InputInjector::InjectMouseEvent(const MouseEventData& event) {
    INPUT input = {};
    input.type = INPUT_MOUSE;

    // Normalize coordinates to 0-65535 range
    input.mi.dx = (event.x * 65535) / screenWidth_;
    input.mi.dy = (event.y * 65535) / screenHeight_;
    input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE;

    switch (event.eventType) {
        case MouseEventData::Type::MOVE:
            input.mi.dwFlags |= MOUSEEVENTF_MOVE;
            break;

        case MouseEventData::Type::LEFT_DOWN:
            input.mi.dwFlags |= MOUSEEVENTF_LEFTDOWN;
            break;

        case MouseEventData::Type::LEFT_UP:
            input.mi.dwFlags |= MOUSEEVENTF_LEFTUP;
            break;

        case MouseEventData::Type::RIGHT_DOWN:
            input.mi.dwFlags |= MOUSEEVENTF_RIGHTDOWN;
            break;

        case MouseEventData::Type::RIGHT_UP:
            input.mi.dwFlags |= MOUSEEVENTF_RIGHTUP;
            break;

        case MouseEventData::Type::MIDDLE_DOWN:
            input.mi.dwFlags |= MOUSEEVENTF_MIDDLEDOWN;
            break;

        case MouseEventData::Type::MIDDLE_UP:
            input.mi.dwFlags |= MOUSEEVENTF_MIDDLEUP;
            break;

        case MouseEventData::Type::WHEEL:
            input.mi.dwFlags |= MOUSEEVENTF_WHEEL;
            input.mi.mouseData = event.wheelDelta;
            break;

        default:
            std::cerr << "[INPUT] Unknown mouse event type" << std::endl;
            return false;
    }

    UINT result = SendInput(1, &input, sizeof(INPUT));
    if (result != 1) {
        std::cerr << "[INPUT] Failed to inject mouse event: " << GetLastError() << std::endl;
        return false;
    }

    return true;
}

bool InputInjector::InjectKeyboardEvent(const KeyboardEventData& event) {
    INPUT input = {};
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = static_cast<WORD>(event.virtualKeyCode);
    input.ki.wScan = static_cast<WORD>(event.scanCode);

    if (event.eventType == KeyboardEventData::Type::KEY_UP) {
        input.ki.dwFlags = KEYEVENTF_KEYUP;
    }

    // Extended keys (arrows, page up/down, etc.)
    if (event.virtualKeyCode >= VK_PRIOR && event.virtualKeyCode <= VK_DOWN) {
        input.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
    }

    UINT result = SendInput(1, &input, sizeof(INPUT));
    if (result != 1) {
        std::cerr << "[INPUT] Failed to inject keyboard event: " << GetLastError() << std::endl;
        return false;
    }

    return true;
}

} // namespace RemoteDesktop
