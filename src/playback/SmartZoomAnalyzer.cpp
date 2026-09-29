#include "SmartZoomAnalyzer.h"
#include <cmath>

namespace ssa::playback {

std::vector<project::CameraKeyframe> SmartZoomAnalyzer::generateKeyframes(
    const project::ProjectMetadata& metadata, 
    int targetWidth, 
    int targetHeight) {
    
    std::vector<project::CameraKeyframe> keyframes;
    
    // Default center camera state (raw video bounds)
    double centerX = metadata.videoWidth / 2.0;
    double centerY = metadata.videoHeight / 2.0;

    project::CameraKeyframe startKf;
    startKf.timestampMs = 0;
    startKf.x = centerX;
    startKf.y = centerY;
    startKf.zoom = 1.0;
    startKf.easing = project::EasingType::Smoothstep;
    keyframes.push_back(startKf);

    if (metadata.mouseEvents.empty()) {
        return keyframes;
    }

    uint64_t startTimestampUs = metadata.mouseEvents.front().timestamp;
    
    struct ClickData {
        qint64 timeMs;
        double x;
        double y;
    };
    
    std::vector<ClickData> clicks;
    bool wasDown = false;
    
    for (const auto& ev : metadata.mouseEvents) {
        bool isDown = (ev.type == input::MouseEventType::Down || ev.type == input::MouseEventType::Drag);
        
        if (isDown && !wasDown) {
            qint64 relativeMs = (static_cast<qint64>(ev.timestamp) - static_cast<qint64>(startTimestampUs)) / 1000;
            
            // Ignore double clicks or extremely rapid clicks (within 500ms)
            if (clicks.empty() || (relativeMs - clicks.back().timeMs) > 500) {
                clicks.push_back({std::max<qint64>(0, relativeMs), ev.x, ev.y});
            }
        }
        wasDown = isDown;
    }
    
    for (size_t i = 0; i < clicks.size(); ++i) {
        const auto& currentClick = clicks[i];
        
        bool isFirstInSequence = (i == 0) || (currentClick.timeMs - clicks[i-1].timeMs > 3000);
        bool isLastInSequence = (i == clicks.size() - 1) || (clicks[i+1].timeMs - currentClick.timeMs > 3000);
        
        double targetZoom = 1.6;
        
        // Calculate visible screen space using the TARGET aspect ratio bounds!
        double visibleWidth = targetWidth / targetZoom;
        double visibleHeight = targetHeight / targetZoom;
        
        double yOffset = visibleHeight * 0.15;
        
        double targetX = currentClick.x;
        double targetY = currentClick.y - yOffset;
        
        // Clamp to prevent camera from showing out-of-bounds black bars
        // The raw video boundaries are metadata.videoWidth and metadata.videoHeight
        targetX = std::clamp(targetX, visibleWidth / 2.0, metadata.videoWidth - visibleWidth / 2.0);
        targetY = std::clamp(targetY, visibleHeight / 2.0, metadata.videoHeight - visibleHeight / 2.0);

        // Helper to find mouse position at a specific time
        auto getMousePos = [&](qint64 targetMs) -> std::pair<double, double> {
            uint64_t targetUs = startTimestampUs + (targetMs * 1000);
            auto it = std::lower_bound(metadata.mouseEvents.begin(), metadata.mouseEvents.end(), targetUs,
                [](const input::MouseEvent& ev, uint64_t t) {
                    return ev.timestamp < t;
                });
            if (it != metadata.mouseEvents.begin() && (it == metadata.mouseEvents.end() || it->timestamp > targetUs)) {
                --it;
            }
            if (it != metadata.mouseEvents.end()) {
                return {it->x, it->y};
            }
            return {targetX, targetY + yOffset};
        };

        if (isFirstInSequence) {
            project::CameraKeyframe pre;
            pre.timestampMs = std::max<qint64>(0, currentClick.timeMs - 1000);
            pre.x = centerX;
            pre.y = centerY;
            pre.zoom = 1.0;
            pre.easing = project::EasingType::Smoothstep;
            keyframes.push_back(pre);
        }
        
        project::CameraKeyframe action;
        action.timestampMs = currentClick.timeMs;
        action.x = targetX;
        action.y = targetY;
        action.zoom = targetZoom;
        action.easing = project::EasingType::Smoothstep;
        keyframes.push_back(action);
        
        if (isLastInSequence) {
            // Follow the mouse for exactly 2.0 seconds (2000ms) after the click
            project::CameraKeyframe hold;
            hold.timestampMs = currentClick.timeMs + 2000;
            
            auto [finalX, finalY] = getMousePos(hold.timestampMs);
            double targetHoldX = std::clamp(finalX, visibleWidth / 2.0, metadata.videoWidth - visibleWidth / 2.0);
            double targetHoldY = std::clamp(finalY - yOffset, visibleHeight / 2.0, metadata.videoHeight - visibleHeight / 2.0);
            
            hold.x = targetHoldX;
            hold.y = targetHoldY;
            hold.zoom = targetZoom;
            hold.easing = project::EasingType::Smoothstep; // Smoothly interpolate over 2s to follow it!
            keyframes.push_back(hold);
            
            project::CameraKeyframe post;
            post.timestampMs = hold.timestampMs + 1000;
            post.x = centerX;
            post.y = centerY;
            post.zoom = 1.0;
            post.easing = project::EasingType::Smoothstep;
            keyframes.push_back(post);
        } else {
            const auto& nextClick = clicks[i+1];
            qint64 holdUntil = nextClick.timeMs - 600; // start moving slightly earlier
            
            if (holdUntil > action.timestampMs) {
                project::CameraKeyframe hold;
                hold.timestampMs = holdUntil;
                
                auto [midpX, midpY] = getMousePos(holdUntil);
                double targetHoldX = std::clamp(midpX, visibleWidth / 2.0, metadata.videoWidth - visibleWidth / 2.0);
                double targetHoldY = std::clamp(midpY - yOffset, visibleHeight / 2.0, metadata.videoHeight - visibleHeight / 2.0);
                
                hold.x = targetHoldX;
                hold.y = targetHoldY;
                hold.zoom = targetZoom;
                hold.easing = project::EasingType::Smoothstep;
                keyframes.push_back(hold);
                
                // Advanced Grouping: Calculate distance
                double dx = nextClick.x - currentClick.x;
                double dy = nextClick.y - currentClick.y;
                double dist = std::sqrt(dx*dx + dy*dy);
                
                // If the next click is very far away (e.g. > 30% of screen width), we insert an arc-pan (zoom out slightly)
                if (dist > metadata.videoWidth * 0.3) {
                    qint64 midTime = (holdUntil + nextClick.timeMs) / 2;
                    double midX = (currentClick.x + nextClick.x) / 2.0;
                    double midY = (currentClick.y + nextClick.y) / 2.0 - yOffset;
                    
                    // Clamp mid point as well
                    double midZoom = 1.2; // Zoom out to 1.2x
                    double midVisWidth = targetWidth / midZoom;
                    double midVisHeight = targetHeight / midZoom;
                    midX = std::clamp(midX, midVisWidth / 2.0, metadata.videoWidth - midVisWidth / 2.0);
                    midY = std::clamp(midY, midVisHeight / 2.0, metadata.videoHeight - midVisHeight / 2.0);
                    
                    project::CameraKeyframe arc;
                    arc.timestampMs = midTime;
                    arc.x = midX;
                    arc.y = midY;
                    arc.zoom = midZoom;
                    arc.easing = project::EasingType::Smoothstep;
                    keyframes.push_back(arc);
                }
            }
        }
    }

    return keyframes;
}

} // namespace ssa::playback
