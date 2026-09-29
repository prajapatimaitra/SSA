#include "platform/interfaces/IDisplayManager.h"

namespace ssa::platform {

class WindowsDisplayManager : public IDisplayManager {
public:
    std::vector<DisplayInfo> enumerateDisplays() override {
        // Stub for Windows
        return {};
    }
};

} // namespace ssa::platform
