#pragma once

#include <QObject>
#include <QTimer>
#include <optional>
#include "project/ProjectMetadata.h"
#include "playback/CameraEngine.h"

namespace ssa::editor {

class PlaybackEngine : public QObject {
    Q_OBJECT

public:
    Q_PROPERTY(QString cursorType READ cursorType NOTIFY cursorUpdated)

    explicit PlaybackEngine(QObject* parent = nullptr);
    ~PlaybackEngine() override;

    void loadMetadata(const project::ProjectMetadata& metadata, int targetWidth = -1, int targetHeight = -1);
    void setTotalDuration(qint64 durationMs);
    // UI Playback control
    Q_INVOKABLE void togglePlayback();
    Q_INVOKABLE void seek(qint64 timeMs);
    void silentSeek(qint64 timeMs); // Seeks without updating the UI video player
    void syncTime(qint64 mediaTimeMs); // Syncs playback engine directly to media player clock

    // Thread-safe pure math evaluation for offline export
    const playback::CameraEngine& cameraEngine() const { return m_cameraEngine; }
    static void getCursorAnchorRatios(const std::string& type, double width, double height, double hx, double hy, double& outAnchorX, double& outAnchorY);
    static double getCursorStyleMultiplier(const std::string& type);
    void evaluateCursorStateOffline(qint64 timeMs, double& outX, double& outY, int& outState, std::string& outType, qint64& outLastClickTimeMs, double& outWidth, double& outHeight, double& outAnchorX, double& outAnchorY, double& outBounceScale, double& outSwayAngle, double& outZoomScale) const;

    void play();
    void pause();

    bool isPlaying() const;
    qint64 currentTime() const;
    qint64 totalDuration() const;

    double cursorX() const;
    double cursorY() const;
    int cursorState() const;
    QString cursorType() const { return m_cursorType; }
    double cursorWidth() const { return m_cursorWidth; }
    double cursorHeight() const { return m_cursorHeight; }
    double cursorHotspotX() const { return m_cursorHotspotX; }
    double cursorHotspotY() const { return m_cursorHotspotY; }
    double cursorAnchorX() const { return m_cursorAnchorX; }
    double cursorAnchorY() const { return m_cursorAnchorY; }
    double cursorBounceScale() const { return m_cursorBounceScale; }
    double cursorSwayAngle() const { return m_cursorSwayAngle; }
    double cursorZoomScale() const { return m_cursorZoomScale; }
    qint64 timeSinceClickMs() const { return m_timeSinceClickMs; }

    double cameraX() const { return m_cameraX; }
    double cameraY() const { return m_cameraY; }
    double cameraZoom() const { return m_cameraZoom; }
    double cameraRotation() const { return m_cameraRotation; }

signals:
    void playbackStateChanged(bool isPlaying);
    void timeUpdated(qint64 currentTime);
    void cursorUpdated(double x, double y, int state, QString type);
    void cursorStateUpdated(double x, double y, int state, QString type, double anchorX, double anchorY, double bounceScale, double swayAngle);
    void cameraUpdated(double x, double y, double zoom, double rotation);
    void durationLoaded(qint64 duration);

private slots:
    void onTick();

private:
    void updateCursorForCurrentTime();

    QTimer m_timer;
    bool m_isPlaying = false;
    qint64 m_currentTime = 0;
    qint64 m_totalDuration = 0;
    qint64 m_lastTickTime = 0; // system time of last tick for delta calc

    double m_cursorX = 0.0;
    double m_cursorY = 0.0;
    int m_cursorState = 0; // 0=Up, 1=Down
    QString m_cursorType = "arrow";
    double m_cursorWidth = 28.0;
    double m_cursorHeight = 40.0;
    double m_cursorHotspotX = 5.0;
    double m_cursorHotspotY = 5.0;
    double m_cursorAnchorX = 0.34;
    double m_cursorAnchorY = 0.24;
    double m_cursorBounceScale = 1.0;
    double m_cursorSwayAngle = 0.0;
    double m_cursorZoomScale = 1.0;
    qint64 m_timeSinceClickMs = 99999;

    std::optional<project::ProjectMetadata> m_metadata;
    
    // Original start timestamp to calculate offsets
    uint64_t m_recordingStartTimestamp = 0; 

    // Camera Engine
    playback::CameraEngine m_cameraEngine;
    double m_cameraX = 0.0;
    double m_cameraY = 0.0;
    double m_cameraZoom = 1.0;
    double m_cameraRotation = 0.0;
};

} // namespace ssa::editor
