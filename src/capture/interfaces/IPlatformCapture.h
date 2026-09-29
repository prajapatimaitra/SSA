#pragma once

#include "media/VideoFrame.h"
#include <functional>
#include <string>

namespace ssa::capture {

struct CaptureConfig {
    std::string displayId;
    std::string outputFilePath;
    std::string systemAudioPath; // If empty, don't capture system audio
    bool isCustomRegion = false;
    int regionX = 0;
    int regionY = 0;
    int regionWidth = 0;
    int regionHeight = 0;
    int fps = 30;
    std::string captureResolution = "Original";
};

class IPlatformCapture {
public:
    virtual ~IPlatformCapture() = default;
    
    // Config contains paths and region settings for the video and system audio
    virtual void startCapture(const CaptureConfig& config) = 0;
    virtual void stopCapture() = 0;
    
    // Callback for when a frame is captured
    virtual void setFrameCallback(std::function<void(const media::VideoFrame&)> callback) = 0;
};

} // namespace ssa::capture
