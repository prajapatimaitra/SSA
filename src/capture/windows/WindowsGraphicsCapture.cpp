#include "capture/interfaces/IPlatformCapture.h"
#include "core/logging/Logger.h"
#include <memory>

namespace ssa::platform {

class WindowsGraphicsCapture : public capture::IPlatformCapture {
public:
    ~WindowsGraphicsCapture() override {}
    
    void startCapture(const std::string& displayId, const std::string& outputFilePath) override {
        core::logging::Logger::info("WindowsGraphicsCapture (Stub): Start Capture on display " + displayId + " to " + outputFilePath);
    }

    void stopCapture() override {}

    void setFrameCallback(std::function<void(const media::VideoFrame&)> callback) override {}
};

std::unique_ptr<capture::IPlatformCapture> createScreenCapture() {
    return std::make_unique<WindowsGraphicsCapture>();
}

} // namespace ssa::platform
