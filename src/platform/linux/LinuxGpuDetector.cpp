#include "platform/interfaces/IGpuDetector.h"

namespace ssa::platform {

class LinuxGpuDetector : public IGpuDetector {
public:
    GpuInfo getPrimaryGpu() override {
        // Stub for Linux
        return {"Unknown Linux GPU", "Unknown", false};
    }
};

} // namespace ssa::platform
