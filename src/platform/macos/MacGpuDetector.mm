#include "platform/interfaces/IGpuDetector.h"
#include <memory>
#import <Metal/Metal.h>

namespace ssa::platform {

class MacGpuDetector : public IGpuDetector {
public:
    GpuInfo getPrimaryGpu() override {
        GpuInfo info;
        info.name = "Unknown Apple GPU";
        info.vendor = "Apple";
        info.isHardwareAccelerated = false;
        
        id<MTLDevice> device = MTLCreateSystemDefaultDevice();
        if (device) {
            info.name = [[device name] UTF8String];
            info.isHardwareAccelerated = true;
        }
        
        return info;
    }
};

std::unique_ptr<IGpuDetector> createGpuDetector() {
    return std::make_unique<MacGpuDetector>();
}

} // namespace ssa::platform
