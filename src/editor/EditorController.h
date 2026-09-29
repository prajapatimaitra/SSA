#pragma once

#include <QObject>
#include <memory>
#include <QString>
#include <QVideoSink>
#include "project/ProjectManager.h"
#include "editor/PlaybackEngine.h"
#include "editor/AutoSaveManager.h"
#include "editor/AudioWaveformExtractor.h"
#include "project/PresetManager.h"

namespace ssa::editor {

class EditorController : public QObject {
    Q_OBJECT
    
public:
    enum class AspectRatio {
        RatioOriginal,
        Ratio16x9,
        Ratio9x16,
        Ratio1x1
    };
    Q_ENUM(AspectRatio)

    Q_PROPERTY(QString projectUuid READ projectUuid NOTIFY projectLoaded)
    Q_PROPERTY(int mouseEventCount READ mouseEventCount NOTIFY projectLoaded)
    Q_PROPERTY(QString projectPath READ projectPath NOTIFY projectLoaded)
    
    Q_PROPERTY(QString videoUrl READ videoUrl NOTIFY projectLoaded)
    Q_PROPERTY(QString webcamUrl READ webcamUrl NOTIFY projectLoaded)
    Q_PROPERTY(QString systemAudioUrl READ systemAudioUrl NOTIFY projectLoaded)
    Q_PROPERTY(QString micAudioUrl READ micAudioUrl NOTIFY projectLoaded)
    Q_PROPERTY(QString subtitleText READ subtitleText NOTIFY subtitleTextChanged)
    Q_PROPERTY(int rawVideoWidth READ rawVideoWidth NOTIFY projectLoaded)
    Q_PROPERTY(int rawVideoHeight READ rawVideoHeight NOTIFY projectLoaded)
    Q_PROPERTY(int videoWidth READ videoWidth NOTIFY aspectRatioChanged)
    Q_PROPERTY(int videoHeight READ videoHeight NOTIFY aspectRatioChanged)
    Q_PROPERTY(AspectRatio aspectRatio READ aspectRatio WRITE setAspectRatio NOTIFY aspectRatioChanged)

    Q_PROPERTY(bool isPlaying READ isPlaying NOTIFY playbackStateChanged)
    Q_PROPERTY(qint64 currentTime READ currentTime NOTIFY timeUpdated)
    Q_PROPERTY(qint64 totalDuration READ totalDuration NOTIFY durationLoaded)
    Q_PROPERTY(QVariantList audioWaveform READ audioWaveform NOTIFY audioWaveformChanged)
    Q_PROPERTY(bool isExtractingWaveform READ isExtractingWaveform NOTIFY audioWaveformChanged)
    Q_PROPERTY(double cursorX READ cursorX NOTIFY cursorUpdated)
    Q_PROPERTY(double cursorY READ cursorY NOTIFY cursorUpdated)
    Q_PROPERTY(int cursorState READ cursorState NOTIFY cursorUpdated)
    Q_PROPERTY(QString cursorType READ cursorType NOTIFY cursorUpdated)
    Q_PROPERTY(double cursorWidth READ cursorWidth NOTIFY cursorUpdated)
    Q_PROPERTY(double cursorHeight READ cursorHeight NOTIFY cursorUpdated)
    Q_PROPERTY(double cursorHotspotX READ cursorHotspotX NOTIFY cursorUpdated)
    Q_PROPERTY(double cursorHotspotY READ cursorHotspotY NOTIFY cursorUpdated)
    Q_PROPERTY(double cursorAnchorX READ cursorAnchorX NOTIFY cursorUpdated)
    Q_PROPERTY(double cursorAnchorY READ cursorAnchorY NOTIFY cursorUpdated)
    Q_PROPERTY(double cursorBounceScale READ cursorBounceScale NOTIFY cursorUpdated)
    Q_PROPERTY(double cursorSwayAngle READ cursorSwayAngle NOTIFY cursorUpdated)
    Q_PROPERTY(double cursorZoomScale READ cursorZoomScale NOTIFY cursorUpdated)
    Q_PROPERTY(qint64 timeSinceClick READ timeSinceClick NOTIFY cursorUpdated)
    Q_PROPERTY(double cameraX READ cameraX NOTIFY cameraUpdated)
    Q_PROPERTY(double cameraY READ cameraY NOTIFY cameraUpdated)
    Q_PROPERTY(double cameraZoom READ cameraZoom NOTIFY cameraUpdated)
    Q_PROPERTY(double cameraRotation READ cameraRotation NOTIFY cameraUpdated)

    Q_PROPERTY(QVideoSink* videoSink READ videoSink WRITE setVideoSink NOTIFY videoSinkChanged)
    Q_PROPERTY(QVideoSink* webcamSink READ webcamSink WRITE setWebcamSink NOTIFY webcamSinkChanged)

    Q_PROPERTY(qint64 trimStart READ trimStart WRITE setTrimStart NOTIFY trimStartChanged)
    Q_PROPERTY(qint64 trimEnd READ trimEnd WRITE setTrimEnd NOTIFY trimEndChanged)

    Q_PROPERTY(QString cursorColor READ cursorColor WRITE setCursorColor NOTIFY cursorColorChanged)
    Q_PROPERTY(double cursorScale READ cursorScale WRITE setCursorScale NOTIFY cursorScaleChanged)
    Q_PROPERTY(QString backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY backgroundColorChanged)
    
    Q_PROPERTY(QVariantList markers READ markers NOTIFY markersChanged)
    
public:
    explicit EditorController(QObject* parent = nullptr);

    qint64 duration() const;
    qint64 playbackTime() const;
    QString videoUrl() const;
    QString webcamUrl() const;
    QString systemAudioUrl() const;
    QString micAudioUrl() const;
    QString subtitleText() const;
    int rawVideoWidth() const;
    int rawVideoHeight() const;
    int videoWidth() const;
    int videoHeight() const;
    
    AspectRatio aspectRatio() const;
    void setAspectRatio(AspectRatio ratio);

    std::optional<project::ProjectMetadata> currentMetadata() const;

    Q_INVOKABLE void loadProject(const QString& path);
    Q_INVOKABLE void loadSubtitles(const QString& vttPath);
    Q_INVOKABLE void togglePlayback();
    Q_INVOKABLE void seek(qint64 timeMs);
    Q_INVOKABLE void syncTime(qint64 mediaTimeMs);
    Q_INVOKABLE void requestWebcamPiP();
    
    Q_INVOKABLE void saveProject();
    Q_INVOKABLE void updateDurationFromMedia(qint64 durationMs);
    Q_INVOKABLE void updateResolutionFromMedia(int width, int height);

    Q_INVOKABLE void saveAsPreset(const QString& presetName);
    Q_INVOKABLE void applyPreset(const QString& presetName);
    Q_INVOKABLE QList<QString> getAvailablePresets() const;
    Q_INVOKABLE QStringList getAvailableWallpapers() const;

    qint64 trimStart() const;
    void setTrimStart(qint64 timeMs);
    qint64 trimEnd() const;
    void setTrimEnd(qint64 timeMs);

    Q_INVOKABLE void addMarker(qint64 timestampMs, const QString& note, const QString& color);
    Q_INVOKABLE void removeMarker(int index);
    Q_INVOKABLE void updateMarkerNote(int index, const QString& note);

    QString cursorColor() const;
    void setCursorColor(const QString& color);
    double cursorScale() const;
    void setCursorScale(double scale);
    QString backgroundColor() const;
    void setBackgroundColor(const QString& color);

    QString projectUuid() const;
    int mouseEventCount() const;
    QString projectPath() const;

    bool isPlaying() const;
    qint64 currentTime() const;
    qint64 totalDuration() const;
    double cursorX() const;
    double cursorY() const;
    int cursorState() const;
    QString cursorType() const;
    double cursorWidth() const;
    double cursorHeight() const;
    double cursorHotspotX() const;
    double cursorHotspotY() const;
    double cursorAnchorX() const;
    double cursorAnchorY() const;
    double cursorBounceScale() const;
    double cursorSwayAngle() const;
    double cursorZoomScale() const;
    qint64 timeSinceClick() const;
    
    QVariantList audioWaveform() const;
    bool isExtractingWaveform() const;
    
    double cameraX() const;
    double cameraY() const;
    double cameraZoom() const;
    double cameraRotation() const;
    
    QVariantList markers() const;

    QVideoSink* videoSink() const;
    void setVideoSink(QVideoSink* sink);
    
    QVideoSink* webcamSink() const;
    void setWebcamSink(QVideoSink* sink);

signals:
    void projectLoaded();
    void playbackStateChanged(bool isPlaying);
    void timeUpdated(qint64 currentTime);
    void durationLoaded(qint64 duration);
    void cursorUpdated();
    void cameraUpdated();
    void videoSinkChanged();
    void webcamSinkChanged();
    void aspectRatioChanged();
    void trimStartChanged();
    void trimEndChanged();
    void cursorColorChanged();
    void cursorScaleChanged();
    void backgroundColorChanged();
    void subtitleTextChanged();
    void audioWaveformChanged();
    void markersChanged();

private:
    project::ProjectManager m_projectManager;
    
    PlaybackEngine m_playbackEngine;
    
    std::optional<project::ProjectMetadata> m_currentProject;
    QString m_projectPath;
    
    QVideoSink* m_videoSink = nullptr;
    QVideoSink* m_webcamSink = nullptr;
    
    AspectRatio m_aspectRatio = AspectRatio::RatioOriginal;
    
    std::unique_ptr<AutoSaveManager> m_autoSaveManager;
    std::unique_ptr<AudioWaveformExtractor> m_waveformExtractor;
    
    struct Subtitle {
        qint64 startTimeMs;
        qint64 endTimeMs;
        QString text;
    };
    std::vector<Subtitle> m_subtitles;
    void parseVttFile(const QString& vttPath);
    void updateSubtitleText();
    QString m_currentSubtitleText;
};

} // namespace ssa::editor
