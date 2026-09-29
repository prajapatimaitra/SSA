#pragma once

namespace ssa::core::coordinates {

class CoordinateMapper {
public:
    static double mapX(double rawX);
    static double mapY(double rawY);
};

} // namespace ssa::core::coordinates
