#pragma once

#include <string>
#include <memory>
#include <QImage>
#include <QString>

namespace ssa::export_engine {

class IMediaDecoder {
public:
    virtual ~IMediaDecoder() = default;

    /**
     * @brief Initializes the decoder with the given video file
     */
    virtual bool initialize(const std::string& videoPath) = 0;

    /**
     * @brief Extracts a frame from the video at the given timestamp
     */
    virtual QImage getFrameAtTime(qint64 timeMs) = 0;
};

std::unique_ptr<IMediaDecoder> createMediaDecoder();

} // namespace ssa::export_engine
