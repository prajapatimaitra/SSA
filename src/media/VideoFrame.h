#pragma once

#include <cstdint>

namespace ssa::media {

enum class PixelFormat {
    UNKNOWN,
    BGRA,
    RGBA,
    NV12,
    P010
};

enum class ColorSpace {
    UNKNOWN,
    SRGB,
    DISPLAY_P3,
    REC709
};

struct VideoFrame {
    uint64_t timestamp; // microseconds or nanoseconds
    int width;
    int height;
    PixelFormat format;
    ColorSpace colorSpace;
    void* nativeBuffer; // CFTypeRef / CVPixelBufferRef on Mac, etc.
};

} // namespace ssa::media
