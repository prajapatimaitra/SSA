#include "PlaybackEngine.h"
#include <QDateTime>

#include "playback/SmartZoomAnalyzer.h"

namespace ssa::editor {

PlaybackEngine::PlaybackEngine(QObject* parent) : QObject(parent) {
    m_timer.setInterval(16); // ~60fps
    connect(&m_timer, &QTimer::timeout, this, &PlaybackEngine::onTick);
}

PlaybackEngine::~PlaybackEngine() {
    m_timer.stop();
}

void PlaybackEngine::loadMetadata(const project::ProjectMetadata& metadata, int targetWidth, int targetHeight) {
    m_metadata = metadata;
    
    // Pin time 0 to the exact first video frame timestamp (recordingStartTimestamp)
    if (metadata.recordingStartTimestamp > 0) {
        m_recordingStartTimestamp = metadata.recordingStartTimestamp;
    } else if (!metadata.mouseEvents.empty()) {
        m_recordingStartTimestamp = metadata.mouseEvents.front().timestamp;
    } else {
        m_recordingStartTimestamp = 0;
    }
    
    if (!metadata.mouseEvents.empty() && metadata.mouseEvents.back().timestamp > m_recordingStartTimestamp) {
        m_totalDuration = (metadata.mouseEvents.back().timestamp - m_recordingStartTimestamp) / 1000;
    } else {
        m_totalDuration = 0;
    }

    int tWidth = (targetWidth > 0) ? targetWidth : metadata.videoWidth;
    int tHeight = (targetHeight > 0) ? targetHeight : metadata.videoHeight;

    // Automatically generate smart zoom keyframes from the raw clicks!
    std::vector<project::CameraKeyframe> keyframes = playback::SmartZoomAnalyzer::generateKeyframes(metadata, tWidth, tHeight);
    m_cameraEngine.loadKeyframes(keyframes);

    emit durationLoaded(m_totalDuration);
    seek(0);
}

void PlaybackEngine::setTotalDuration(qint64 durationMs) {
    m_totalDuration = durationMs;
    emit durationLoaded(m_totalDuration);
}

void PlaybackEngine::play() {
    if (m_totalDuration == 0) return;
    if (m_currentTime >= m_totalDuration) m_currentTime = 0; // wrap around
    
    m_isPlaying = true;
    m_lastTickTime = QDateTime::currentMSecsSinceEpoch();
    m_timer.start();
    emit playbackStateChanged(true);
}

void PlaybackEngine::pause() {
    m_isPlaying = false;
    m_timer.stop();
    emit playbackStateChanged(false);
}

void PlaybackEngine::togglePlayback() {
    if (m_isPlaying) pause();
    else play();
}

void PlaybackEngine::seek(qint64 timeMs) {
    m_currentTime = std::clamp(timeMs, Q_INT64_C(0), m_totalDuration);
    updateCursorForCurrentTime();
    
    // Evaluate camera
    auto camState = m_cameraEngine.evaluateAtTime(m_currentTime);
    m_cameraX = camState.x;
    m_cameraY = camState.y;
    m_cameraZoom = camState.zoom;
    m_cameraRotation = camState.rotation;
    
    emit cameraUpdated(m_cameraX, m_cameraY, m_cameraZoom, m_cameraRotation);
    emit timeUpdated(m_currentTime);
}

void PlaybackEngine::silentSeek(qint64 timeMs) {
    m_currentTime = std::clamp(timeMs, Q_INT64_C(0), m_totalDuration);
    updateCursorForCurrentTime();
    
    // Evaluate camera
    auto camState = m_cameraEngine.evaluateAtTime(m_currentTime);
    m_cameraX = camState.x;
    m_cameraY = camState.y;
    m_cameraZoom = camState.zoom;
    m_cameraRotation = camState.rotation;
    // We intentionally DO NOT emit signals here to prevent the UI from thrashing during fast offline export!
}

void PlaybackEngine::syncTime(qint64 mediaTimeMs) {
    if (std::abs(m_currentTime - mediaTimeMs) > 1) {
        m_currentTime = std::clamp(mediaTimeMs, Q_INT64_C(0), m_totalDuration);
        updateCursorForCurrentTime();
        auto camState = m_cameraEngine.evaluateAtTime(m_currentTime);
        m_cameraX = camState.x;
        m_cameraY = camState.y;
        m_cameraZoom = camState.zoom;
        m_cameraRotation = camState.rotation;
        emit cameraUpdated(m_cameraX, m_cameraY, m_cameraZoom, m_cameraRotation);
        emit timeUpdated(m_currentTime);
    }
}

bool PlaybackEngine::isPlaying() const { return m_isPlaying; }
qint64 PlaybackEngine::currentTime() const { return m_currentTime; }
qint64 PlaybackEngine::totalDuration() const { return m_totalDuration; }

double PlaybackEngine::cursorX() const { return m_cursorX; }
double PlaybackEngine::cursorY() const { return m_cursorY; }
int PlaybackEngine::cursorState() const { return m_cursorState; }

void PlaybackEngine::onTick() {
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    qint64 delta = now - m_lastTickTime;
    m_lastTickTime = now;

    m_currentTime += delta;

    if (m_currentTime >= m_totalDuration) {
        m_currentTime = m_totalDuration;
        pause();
    }

    updateCursorForCurrentTime();

    // Evaluate camera
    auto camState = m_cameraEngine.evaluateAtTime(m_currentTime);
    m_cameraX = camState.x;
    m_cameraY = camState.y;
    m_cameraZoom = camState.zoom;
    m_cameraRotation = camState.rotation;
    
    emit cameraUpdated(m_cameraX, m_cameraY, m_cameraZoom, m_cameraRotation);

    emit timeUpdated(m_currentTime);
}

void PlaybackEngine::getCursorAnchorRatios(const std::string& type, double width, double height, double hx, double hy, double& outAnchorX, double& outAnchorY) {
    if (type == "arrow" || type == "pointer") {
        outAnchorX = 0.14;
        outAnchorY = 0.06;
    } else if (type == "pointingHand") {
        outAnchorX = 0.40;
        outAnchorY = 0.10;
    } else if (type == "iBeam" || type == "iBeamCursorForVerticalLayout") {
        outAnchorX = 0.50;
        outAnchorY = 0.44;
    } else if (type == "closedHand") {
        outAnchorX = 0.50;
        outAnchorY = 0.46;
    } else if (type == "openHand") {
        outAnchorX = 0.55;
        outAnchorY = 0.57;
    } else if (type == "crosshair") {
        outAnchorX = 0.50;
        outAnchorY = 0.46;
    } else if (type == "contextualMenu") {
        outAnchorX = 0.12;
        outAnchorY = 0.05;
    } else if (type == "zoomIn") {
        outAnchorX = 0.42;
        outAnchorY = 0.39;
    } else if (type == "zoomOut") {
        outAnchorX = 0.43;
        outAnchorY = 0.39;
    } else if (type == "dragCopy" || type == "busyButClickable" || type == "operationNotAllowed") {
        outAnchorX = 0.23;
        outAnchorY = 0.00;
    } else if (type.find("resize") != std::string::npos || type.find("Resize") != std::string::npos ||
               type.find("window") != std::string::npos || type.find("Window") != std::string::npos) {
        outAnchorX = 0.50;
        outAnchorY = 0.50;
    } else if (width > 0.0 && height > 0.0 && hx > 0.0 && hy > 0.0) {
        outAnchorX = std::clamp(hx / width, 0.0, 1.0);
        outAnchorY = std::clamp(hy / height, 0.0, 1.0);
    } else {
        outAnchorX = 0.14;
        outAnchorY = 0.06;
    }
}

double PlaybackEngine::getCursorStyleMultiplier(const std::string& type) {
    if (type == "pointingHand") return 1.10;
    if (type == "crosshair") return 0.90;
    if (type == "iBeam" || type == "iBeamCursorForVerticalLayout") return 1.00;
    return 1.00;
}

void PlaybackEngine::evaluateCursorStateOffline(qint64 timeMs, double& outX, double& outY, int& outState, std::string& outType, qint64& outLastClickTimeMs, double& outWidth, double& outHeight, double& outAnchorX, double& outAnchorY, double& outBounceScale, double& outSwayAngle, double& outZoomScale) const {
    double camZoom = m_cameraEngine.evaluateAtTime(timeMs).zoom;
    // Magnifies cursor size proportionally during camera zoom-in (0.95x zoom-scaling)
    // At 1.8x camera zoom, cursor size grows to 1.76x baseline (50px x 70px), making the click bounce press shrinking high-impact & prominent.
    outZoomScale = 1.0 + (std::max(1.0, camZoom) - 1.0) * 0.95;

    if (!m_metadata || m_metadata->mouseEvents.empty()) {
        outX = 0;
        outY = 0;
        outState = 0;
        outType = "arrow";
        outLastClickTimeMs = 0;
        outWidth = 28.0;
        outHeight = 40.0;
        outAnchorX = 0.34;
        outAnchorY = 0.24;
        outBounceScale = 1.0;
        outSwayAngle = 0.0;
        return;
    }

    uint64_t targetTimeUs = m_recordingStartTimestamp + (timeMs * 1000);
    
    // Binary search for the closest event BEFORE or AT targetTimeUs
    auto it = std::lower_bound(m_metadata->mouseEvents.begin(), m_metadata->mouseEvents.end(), targetTimeUs,
        [](const input::MouseEvent& ev, uint64_t t) {
            return ev.timestamp < t;
        });

    if (it != m_metadata->mouseEvents.begin()) {
        if (it == m_metadata->mouseEvents.end() || it->timestamp > targetTimeUs) {
            --it; // The event just before targetTimeUs
        }
    } else if (it != m_metadata->mouseEvents.end() && it->timestamp > targetTimeUs) {
        // Before the first event
        outX = it->x;
        outY = it->y;
        outState = (it->type == input::MouseEventType::Down) ? 1 : 0;
        outType = it->cursorType;
        outLastClickTimeMs = (outState == 1) ? (it->timestamp - m_recordingStartTimestamp) / 1000 : 0;
        outWidth = it->cursorWidth;
        outHeight = it->cursorHeight;
        getCursorAnchorRatios(outType, outWidth, outHeight, it->cursorHotspotX, it->cursorHotspotY, outAnchorX, outAnchorY);
        outBounceScale = 1.0;
        outSwayAngle = 0.0;
        return;
    }

    if (it != m_metadata->mouseEvents.end()) {
        outX = it->x;
        outY = it->y;
        
        // Find the most recent click state and cursor type by searching backwards from 'it'
        outState = 0;
        outType = it->cursorType;
        outWidth = it->cursorWidth;
        outHeight = it->cursorHeight;
        getCursorAnchorRatios(outType, outWidth, outHeight, it->cursorHotspotX, it->cursorHotspotY, outAnchorX, outAnchorY);
        
        auto stateIt = it;
        outLastClickTimeMs = 0;
        while (true) {
            if (stateIt->type == input::MouseEventType::Down) {
                outState = 1;
                outLastClickTimeMs = (stateIt->timestamp - m_recordingStartTimestamp) / 1000;
                break;
            }
            if (stateIt == m_metadata->mouseEvents.begin()) break;
            --stateIt;
        }

        // Sinusoidal Click Bounce Press Compression (30% Compression over 450ms Window)
        if (outLastClickTimeMs > 0 && timeMs >= outLastClickTimeMs) {
            qint64 timeSinceClick = timeMs - outLastClickTimeMs;
            if (timeSinceClick < 450) { // Extended 450ms bounce window for smooth, clear visual compression
                double t = timeSinceClick / 450.0;
                outBounceScale = std::max(0.70, 1.0 - std::sin(t * M_PI) * 0.30); // Compresses height by 30% on press contact
            } else {
                outBounceScale = 1.0;
            }
        } else {
            outBounceScale = 1.0;
        }

        // Motion Sway Physics Velocity Calculation
        double vx = 0.0;
        if (it != m_metadata->mouseEvents.begin()) {
            auto prevIt = std::prev(it);
            double dt = (it->timestamp - prevIt->timestamp) / 1000000.0; // seconds
            if (dt > 0.001) {
                vx = (it->x - prevIt->x) / dt;
            }
        }
        // Sway rotation in degrees (clamped to [-15 deg, +15 deg])
        outSwayAngle = std::clamp(vx * 0.008, -15.0, 15.0);
    }
}

void PlaybackEngine::updateCursorForCurrentTime() {
    std::string type;
    qint64 lastClickTimeMs = 0;
    evaluateCursorStateOffline(m_currentTime, m_cursorX, m_cursorY, m_cursorState, type, lastClickTimeMs, m_cursorWidth, m_cursorHeight, m_cursorAnchorX, m_cursorAnchorY, m_cursorBounceScale, m_cursorSwayAngle, m_cursorZoomScale);
    m_cursorType = QString::fromStdString(type);
    m_timeSinceClickMs = (lastClickTimeMs > 0 && m_currentTime >= lastClickTimeMs) ? (m_currentTime - lastClickTimeMs) : 99999;
    emit cursorUpdated(m_cursorX, m_cursorY, m_cursorState, m_cursorType);
    emit cursorStateUpdated(m_cursorX, m_cursorY, m_cursorState, m_cursorType, m_cursorAnchorX, m_cursorAnchorY, m_cursorBounceScale, m_cursorSwayAngle);
}
} // namespace ssa::editor
