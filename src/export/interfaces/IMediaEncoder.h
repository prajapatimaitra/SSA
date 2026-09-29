#pragma once

#include <string>
#include <QImage>
#include <memory>

namespace ssa::export_engine {

struct EncoderConfig {
    std::string codec = "h264";
    std::string quality = "High";
    bool hardwareAccel = true;
};

class IMediaEncoder {
public:
    virtual ~IMediaEncoder() = default;

    /**
     * Initializes the encoder.
     * @param outputPath The absolute path to save the .mp4 file.
     * @param width The width of the video.
     * @param height The height of the video.
     * @param fps The frames per second.
     * @param config The encoder configuration.
     * @return True if successful.
     */
    virtual bool initialize(const std::string& outputPath, int width, int height, int fps, const EncoderConfig& config) = 0;

    /**
     * Encodes and appends a single frame to the video.
     * @param frame The QImage containing the composited frame.
     * @param ptsMicroseconds The presentation timestamp in microseconds.
     * @return True if successful.
     */
    virtual bool appendFrame(const QImage& frame, uint64_t ptsMicroseconds) = 0;

    /**
     * Finalizes and closes the video file.
     */
    virtual void finish() = 0;
};

std::unique_ptr<IMediaEncoder> createMediaEncoder();

} // namespace ssa::export_engine
