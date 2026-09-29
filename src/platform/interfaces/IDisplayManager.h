#pragma once

#include <vector>
#include <string>

namespace ssa::platform {

struct DisplayInfo {
    std::string id;
    std::string name;
    int x;
    int y;
    int width;
    int height;
    double scaleFactor;
    bool isPrimary;
};

class IDisplayManager {
public:
    virtual ~IDisplayManager() = default;
    virtual std::vector<DisplayInfo> enumerateDisplays() = 0;
};

} // namespace ssa::platform
