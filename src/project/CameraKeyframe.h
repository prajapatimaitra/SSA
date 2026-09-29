#pragma once

#include <QtGlobal>

namespace ssa::project {

enum class EasingType {
    Linear,
    EaseInOut,
    Smoothstep
};

struct CameraKeyframe {
    qint64 timestampMs = 0;
    
    // The point in the video that should be at the center of the camera view
    double x = 0.0;
    double y = 0.0;
    
    // Zoom factor. 1.0 = normal size, >1.0 = zoomed in
    double zoom = 1.0;
    
    // Rotation in degrees
    double rotation = 0.0;
    
    EasingType easing = EasingType::EaseInOut;
};

} // namespace ssa::project
