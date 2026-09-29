#include "CoordinateMapper.h"

namespace ssa::core::coordinates {

double CoordinateMapper::mapX(double rawX) {
    // Basic stub: in a multi-monitor/retina scenario, this needs to calculate 
    // exact relative offsets and DPI scale factors. For now, 1:1 map.
    return rawX;
}

double CoordinateMapper::mapY(double rawY) {
    // Basic stub
    return rawY;
}

} // namespace ssa::core::coordinates
