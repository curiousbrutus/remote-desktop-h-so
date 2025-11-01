#include "common/compression.h"
#include <windows.h>
#include <gdiplus.h>
#include <iostream>

#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;

namespace RemoteDesktop {

// GDI+ encoder/decoder helper
static int GetEncoderClsid(const WCHAR* format, CLSID* pClsid) {
    UINT num = 0;
    UINT size = 0;

    GetImageEncodersSize(&num, &size);
    if (size == 0) return -1;

    ImageCodecInfo* pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
    if (pImageCodecInfo == NULL) return -1;

    GetImageEncoders(num, size, pImageCodecInfo);

    for (UINT j = 0; j < num; ++j) {
        if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0) {
            *pClsid = pImageCodecInfo[j].Clsid;
            free(pImageCodecInfo);
            return j;
        }
    }

    free(pImageCodecInfo);
    return -1;
}

bool Compression::CompressToJPEG(
    const uint8_t* bgraData,
    uint32_t width,
    uint32_t height,
    uint32_t quality,
    std::vector<uint8_t>& outJpegData)
{
    // Initialize GDI+
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    // Create bitmap from raw data
    Bitmap bitmap(width, height, width * 4, PixelFormat32bppARGB, (BYTE*)bgraData);

    // Get JPEG encoder
    CLSID jpegClsid;
    if (GetEncoderClsid(L"image/jpeg", &jpegClsid) < 0) {
        GdiplusShutdown(gdiplusToken);
        return false;
    }

    // Set quality parameter
    EncoderParameters encoderParams;
    encoderParams.Count = 1;
    encoderParams.Parameter[0].Guid = EncoderQuality;
    encoderParams.Parameter[0].Type = EncoderParameterValueTypeLong;
    encoderParams.Parameter[0].NumberOfValues = 1;
    encoderParams.Parameter[0].Value = &quality;

    // Save to memory stream
    IStream* stream = nullptr;
    CreateStreamOnHGlobal(NULL, TRUE, &stream);

    Status status = bitmap.Save(stream, &jpegClsid, &encoderParams);
    
    if (status != Ok) {
        stream->Release();
        GdiplusShutdown(gdiplusToken);
        return false;
    }

    // Get stream data
    STATSTG statstg;
    stream->Stat(&statstg, STATFLAG_DEFAULT);
    ULONG dataSize = (ULONG)statstg.cbSize.QuadPart;

    outJpegData.resize(dataSize);
    
    LARGE_INTEGER li = {};
    stream->Seek(li, STREAM_SEEK_SET, NULL);
    
    ULONG bytesRead = 0;
    stream->Read(outJpegData.data(), dataSize, &bytesRead);

    stream->Release();
    GdiplusShutdown(gdiplusToken);

    return bytesRead == dataSize;
}

bool Compression::DecompressFromJPEG(
    const uint8_t* jpegData,
    size_t jpegSize,
    std::vector<uint8_t>& outBgraData,
    uint32_t& outWidth,
    uint32_t& outHeight)
{
    // Initialize GDI+
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    // Create stream from JPEG data
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, jpegSize);
    if (!hMem) {
        GdiplusShutdown(gdiplusToken);
        return false;
    }

    void* pMem = GlobalLock(hMem);
    memcpy(pMem, jpegData, jpegSize);
    GlobalUnlock(hMem);

    IStream* stream = nullptr;
    CreateStreamOnHGlobal(hMem, TRUE, &stream);

    // Load bitmap from stream
    Bitmap* bitmap = Bitmap::FromStream(stream);
    stream->Release();

    if (!bitmap || bitmap->GetLastStatus() != Ok) {
        delete bitmap;
        GdiplusShutdown(gdiplusToken);
        return false;
    }

    outWidth = bitmap->GetWidth();
    outHeight = bitmap->GetHeight();

    // Lock bitmap data
    BitmapData bitmapData;
    Rect rect(0, 0, outWidth, outHeight);
    
    if (bitmap->LockBits(&rect, ImageLockModeRead, PixelFormat32bppARGB, &bitmapData) != Ok) {
        delete bitmap;
        GdiplusShutdown(gdiplusToken);
        return false;
    }

    // Copy to output buffer
    size_t dataSize = outWidth * outHeight * 4;
    outBgraData.resize(dataSize);
    memcpy(outBgraData.data(), bitmapData.Scan0, dataSize);

    bitmap->UnlockBits(&bitmapData);
    delete bitmap;
    GdiplusShutdown(gdiplusToken);

    return true;
}

} // namespace RemoteDesktop
