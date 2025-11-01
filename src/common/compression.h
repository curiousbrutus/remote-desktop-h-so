#pragma once
#include <vector>
#include <cstdint>

namespace RemoteDesktop {

class Compression {
public:
    // Compress raw BGRA bitmap to JPEG
    static bool CompressToJPEG(
        const uint8_t* bgraData,
        uint32_t width,
        uint32_t height,
        uint32_t quality, // 1-100
        std::vector<uint8_t>& outJpegData
    );

    // Decompress JPEG to raw BGRA bitmap
    static bool DecompressFromJPEG(
        const uint8_t* jpegData,
        size_t jpegSize,
        std::vector<uint8_t>& outBgraData,
        uint32_t& outWidth,
        uint32_t& outHeight
    );
};

} // namespace RemoteDesktop
