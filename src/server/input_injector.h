#pragma once
#include "common/protocol.h"

namespace RemoteDesktop {

class InputInjector {
public:
    InputInjector();
    ~InputInjector();

    // Inject mouse event
    bool InjectMouseEvent(const MouseEventData& event);
    
    // Inject keyboard event
    bool InjectKeyboardEvent(const KeyboardEventData& event);

private:
    int screenWidth_;
    int screenHeight_;
};

} // namespace RemoteDesktop
