#include "CameraEngine.h"
#include <algorithm>
#include <cmath>

namespace ssa::playback {

CameraEngine::CameraEngine() {}

void CameraEngine::loadKeyframes(const std::vector<project::CameraKeyframe>& keyframes) {
    m_keyframes = keyframes;
    
    // Ensure keyframes are sorted by timestamp
    std::sort(m_keyframes.begin(), m_keyframes.end(), 
        [](const project::CameraKeyframe& a, const project::CameraKeyframe& b) {
            return a.timestampMs < b.timestampMs;
        });
}

void CameraEngine::clear() {
    m_keyframes.clear();
}

CameraState CameraEngine::evaluateAtTime(qint64 timeMs) const {
    if (m_keyframes.empty()) {
        return CameraState{}; // Default identity camera
    }

    // If before first keyframe, clamp to first
    if (timeMs <= m_keyframes.front().timestampMs) {
        const auto& kf = m_keyframes.front();
        return CameraState{kf.x, kf.y, kf.zoom, kf.rotation};
    }

    // If after last keyframe, clamp to last
    if (timeMs >= m_keyframes.back().timestampMs) {
        const auto& kf = m_keyframes.back();
        return CameraState{kf.x, kf.y, kf.zoom, kf.rotation};
    }

    // Find surrounding keyframes
    auto it = std::upper_bound(m_keyframes.begin(), m_keyframes.end(), timeMs,
        [](qint64 t, const project::CameraKeyframe& kf) {
            return t < kf.timestampMs;
        });

    auto nextKf = *it;
    auto prevKf = *(it - 1);

    // Calculate normalized progress 't' between the two keyframes (0.0 to 1.0)
    double range = static_cast<double>(nextKf.timestampMs - prevKf.timestampMs);
    double progress = static_cast<double>(timeMs - prevKf.timestampMs) / range;

    CameraState state;
    // We use prevKf's easing function for the transition to nextKf
    state.x = interpolate(prevKf.x, nextKf.x, progress, prevKf.easing);
    state.y = interpolate(prevKf.y, nextKf.y, progress, prevKf.easing);
    state.zoom = interpolate(prevKf.zoom, nextKf.zoom, progress, prevKf.easing);
    state.rotation = interpolate(prevKf.rotation, nextKf.rotation, progress, prevKf.easing);

    return state;
}

double CameraEngine::interpolate(double start, double end, double t, project::EasingType easing) const {
    double easedT = t;
    
    switch (easing) {
        case project::EasingType::Linear:
            easedT = t;
            break;
        case project::EasingType::EaseInOut:
            // Standard sine ease-in-out
            easedT = -(std::cos(M_PI * t) - 1.0) / 2.0;
            break;
        case project::EasingType::Smoothstep:
            easedT = smoothstep(0.0, 1.0, t);
            break;
    }
    
    return start + (end - start) * easedT;
}

double CameraEngine::smoothstep(double edge0, double edge1, double x) const {
    // Clamp x to [0, 1]
    double t = std::clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0);
    // Smoothstep polynomial
    return t * t * (3.0 - 2.0 * t);
}

} // namespace ssa::playback
