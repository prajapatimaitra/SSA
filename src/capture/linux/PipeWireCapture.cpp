#include "capture/interfaces/IPlatformCapture.h"
#include "core/logging/Logger.h"
#include <memory>

namespace ssa::platform {

class PipeWireCapture : public capture::IPlatformCapture {
public:
    ~PipeWireCapture() override {}
    
    void startCapture(const std::string& displayId, const std::string& outputFilePath) override {
        core::logging::Logger::info("PipeWireCapture (Stub): Start Capture on display " + displayId + " to " + outputFilePath);
    }

    void stopCapture() override {}

    void setFrameCallback(std::function<void(const media::VideoFrame&)> callback) override {}
};

std::unique_ptr<capture::IPlatformCapture> createScreenCapture() {
    return std::make_unique<PipeWireCapture>();
}

} // namespace ssa::platform
