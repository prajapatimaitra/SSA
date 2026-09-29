#pragma once

#include <vector>
#include "project/CameraKeyframe.h"

namespace ssa::playback {

struct CameraState {
    double x = 0.0;
    double y = 0.0;
    double zoom = 1.0;
    double rotation = 0.0;
};

class CameraEngine {
public:
    CameraEngine();

    void loadKeyframes(const std::vector<project::CameraKeyframe>& keyframes);
    void clear();

    // Evaluates the interpolated state for a given timestamp
    CameraState evaluateAtTime(qint64 timeMs) const;

private:
    std::vector<project::CameraKeyframe> m_keyframes;

    // Helper functions
    double interpolate(double start, double end, double t, project::EasingType easing) const;
    double smoothstep(double edge0, double edge1, double x) const;
};

} // namespace ssa::playback
