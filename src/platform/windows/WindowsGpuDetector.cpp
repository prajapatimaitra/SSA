#include "platform/interfaces/IGpuDetector.h"

namespace ssa::platform {

class WindowsGpuDetector : public IGpuDetector {
public:
    GpuInfo getPrimaryGpu() override {
        // Stub for Windows
        return {"Unknown Windows GPU", "Unknown", false};
    }
};

} // namespace ssa::platform
