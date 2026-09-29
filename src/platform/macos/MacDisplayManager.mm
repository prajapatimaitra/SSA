#include "platform/interfaces/IDisplayManager.h"
#include <memory>
#import <CoreGraphics/CoreGraphics.h>

namespace ssa::platform {

class MacDisplayManager : public IDisplayManager {
public:
    std::vector<DisplayInfo> enumerateDisplays() override {
        std::vector<DisplayInfo> displays;
        
        uint32_t displayCount = 0;
        CGGetActiveDisplayList(0, nullptr, &displayCount);
        
        if (displayCount == 0) return displays;
        
        std::vector<CGDirectDisplayID> displayIds(displayCount);
        CGGetActiveDisplayList(displayCount, displayIds.data(), &displayCount);
        
        for (uint32_t i = 0; i < displayCount; ++i) {
            CGDirectDisplayID displayId = displayIds[i];
            CGRect bounds = CGDisplayBounds(displayId);
            
            DisplayInfo info;
            info.id = std::to_string(displayId);
            info.name = "Display " + std::to_string(i + 1);
            info.x = bounds.origin.x;
            info.y = bounds.origin.y;
            info.width = bounds.size.width;
            info.height = bounds.size.height;
            info.scaleFactor = 2.0; // Simplified for now
            info.isPrimary = CGDisplayIsMain(displayId);
            
            displays.push_back(info);
        }
        
        return displays;
    }
};

std::unique_ptr<IDisplayManager> createDisplayManager() {
    return std::make_unique<MacDisplayManager>();
}

} // namespace ssa::platform
