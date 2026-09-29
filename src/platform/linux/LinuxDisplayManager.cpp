#include "platform/interfaces/IDisplayManager.h"

namespace ssa::platform {

class LinuxDisplayManager : public IDisplayManager {
public:
    std::vector<DisplayInfo> enumerateDisplays() override {
        // Stub for Linux
        return {};
    }
};

} // namespace ssa::platform
