#include "platform/interfaces/ICapabilityDetector.h"
#include <memory>
#import <ApplicationServices/ApplicationServices.h>
#import <CoreGraphics/CoreGraphics.h>

namespace ssa::platform {

class MacCapabilityDetector : public ICapabilityDetector {
public:
    bool hasScreenRecordingPermission() override {
        // Simple stub: assume true for now, or check via CGPreflightScreenCaptureAccess in newer macOS
        if (@available(macOS 10.15, *)) {
            return CGPreflightScreenCaptureAccess();
        }
        return true;
    }
    
    bool hasInputMonitoringPermission() override {
        // Stub
        return true;
    }
    
    void requestScreenRecordingPermission() override {
        if (@available(macOS 10.15, *)) {
            CGRequestScreenCaptureAccess();
        }
    }
    
    void requestInputMonitoringPermission() override {
        // Stub
    }
};

std::unique_ptr<ICapabilityDetector> createCapabilityDetector() {
    return std::make_unique<MacCapabilityDetector>();
}

} // namespace ssa::platform
