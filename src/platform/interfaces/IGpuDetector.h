#pragma once

#include <string>

namespace ssa::platform {

struct GpuInfo {
    std::string name;
    std::string vendor;
    bool isHardwareAccelerated;
};

class IGpuDetector {
public:
    virtual ~IGpuDetector() = default;
    virtual GpuInfo getPrimaryGpu() = 0;
};

} // namespace ssa::platform
