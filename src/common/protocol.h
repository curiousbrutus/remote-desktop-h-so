#pragma once
#include <cstdint>

namespace RemoteDesktop {

// Packet types
enum class PacketType : uint32_t {
    SCREEN_FRAME = 1,      // Server -> Client: Screen capture data
    MOUSE_INPUT = 2,       // Client -> Server: Mouse movement/click
    KEYBOARD_INPUT = 3,    // Client -> Server: Keyboard input
    DISCONNECT = 4,        // Either: Connection termination
    HANDSHAKE = 5,         // Initial connection
};

// Packet header structure
#pragma pack(push, 1)
struct PacketHeader {
    PacketType type;
    uint32_t dataSize;     // Size of data following header
};
#pragma pack(pop)

// Screen frame packet
struct ScreenFrameData {
    uint32_t width;
    uint32_t height;
    uint32_t compressionQuality; // JPEG quality (1-100)
    // Followed by compressed JPEG data
};

// Mouse event packet
struct MouseEventData {
    enum class Type : uint8_t {
        MOVE = 0,
        LEFT_DOWN = 1,
        LEFT_UP = 2,
        RIGHT_DOWN = 3,
        RIGHT_UP = 4,
        MIDDLE_DOWN = 5,
        MIDDLE_UP = 6,
        WHEEL = 7
    };
    
    Type eventType;
    int32_t x;
    int32_t y;
    int32_t wheelDelta; // Only for WHEEL events
};

// Keyboard event packet
struct KeyboardEventData {
    enum class Type : uint8_t {
        KEY_DOWN = 0,
        KEY_UP = 1
    };
    
    Type eventType;
    uint32_t virtualKeyCode;
    uint32_t scanCode;
};

// Connection settings
constexpr uint16_t DEFAULT_PORT = 5900; // RDP uses 3389, VNC uses 5900
constexpr uint32_t MAX_PACKET_SIZE = 10 * 1024 * 1024; // 10MB max
constexpr uint32_t SCREEN_FPS = 30; // Target framerate
constexpr uint32_t JPEG_QUALITY = 75; // Default JPEG quality

} // namespace RemoteDesktop
