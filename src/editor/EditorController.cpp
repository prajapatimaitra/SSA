#include "EditorController.h"
#include "editor/AutoSaveManager.h"
#include "editor/AudioWaveformExtractor.h"
#include "core/logging/Logger.h"
#include "project/MetadataSerializer.h"
#include <QFile>
#include <iostream>
#include <QQmlContext>
#include <QDir>

namespace ssa::editor {

EditorController::EditorController(QObject* parent)
    : QObject(parent), m_playbackEngine(this) 
{
    m_autoSaveManager = std::make_unique<AutoSaveManager>(this);
    m_waveformExtractor = std::make_unique<AudioWaveformExtractor>(this);

    connect(m_waveformExtractor.get(), &AudioWaveformExtractor::waveformReady, this, &EditorController::audioWaveformChanged);
    connect(m_waveformExtractor.get(), &AudioWaveformExtractor::extractionStateChanged, this, &EditorController::audioWaveformChanged);

    connect(&m_playbackEngine, &PlaybackEngine::playbackStateChanged, this, &EditorController::playbackStateChanged);
    connect(&m_playbackEngine, &PlaybackEngine::timeUpdated, this, [this](qint64 timeMs) {
        updateSubtitleText();
        emit timeUpdated(timeMs);
    });
    connect(&m_playbackEngine, &PlaybackEngine::durationLoaded, this, &EditorController::durationLoaded);
    connect(&m_playbackEngine, &PlaybackEngine::cursorUpdated, this, [this](double x, double y, int state){
        emit cursorUpdated();
        emit cameraUpdated();
    });
    connect(&m_playbackEngine, &PlaybackEngine::cursorStateUpdated, this, [this](){
        emit cursorUpdated();
        emit cameraUpdated();
    });
    connect(&m_playbackEngine, &PlaybackEngine::cameraUpdated, this, [this](double x, double y, double zoom, double rotation){
        emit cameraUpdated();
    });
}

std::optional<project::ProjectMetadata> EditorController::currentMetadata() const {
    return m_currentProject;
}

QVariantList EditorController::audioWaveform() const {
    return m_waveformExtractor ? m_waveformExtractor->waveform() : QVariantList();
}

bool EditorController::isExtractingWaveform() const {
    return m_waveformExtractor ? m_waveformExtractor->isExtracting() : false;
}

void EditorController::loadSubtitles(const QString& vttPath) {
    parseVttFile(vttPath);
    core::logging::Logger::info("EditorController: Loaded late-arriving subtitles.");
}



QString EditorController::subtitleText() const {
    return m_currentSubtitleText;
}

void EditorController::updateSubtitleText() {
    qint64 currentMs = currentTime();
    QString newText = "";
    
    for (const auto& sub : m_subtitles) {
        if (currentMs >= sub.startTimeMs && currentMs <= sub.endTimeMs) {
            newText = sub.text;
            break;
        }
    }
    
    if (newText != m_currentSubtitleText) {
        m_currentSubtitleText = newText;
        emit subtitleTextChanged();
    }
}

void EditorController::parseVttFile(const QString& vttPath) {
    m_subtitles.clear();
    QFile file(vttPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return; // No subtitles available
    }
    
    QTextStream in(&file);
    QString line = in.readLine();
    if (!line.startsWith("WEBVTT")) return; // Invalid format
    
    auto parseTimeMs = [](const QString& timeStr) -> qint64 {
        // format: 00:00:02.000
        QStringList parts = timeStr.split(':');
        if (parts.size() != 3) return 0;
        
        qint64 hours = parts[0].toLongLong();
        qint64 minutes = parts[1].toLongLong();
        
        QStringList secParts = parts[2].split('.');
        qint64 seconds = secParts[0].toLongLong();
        qint64 ms = secParts.size() > 1 ? secParts[1].toLongLong() : 0;
        
        return (hours * 3600 + minutes * 60 + seconds) * 1000 + ms;
    };
    
    Subtitle currentSub;
    bool readingText = false;
    
    while (!in.atEnd()) {
        line = in.readLine().trimmed();
        if (line.isEmpty()) {
            if (readingText) {
                m_subtitles.push_back(currentSub);
                currentSub.text = "";
                readingText = false;
            }
            continue;
        }
        
        if (line.contains("-->")) {
            QStringList times = line.split("-->");
            if (times.size() == 2) {
                currentSub.startTimeMs = parseTimeMs(times[0].trimmed());
                currentSub.endTimeMs = parseTimeMs(times[1].trimmed());
                readingText = true;
            }
        } else if (readingText) {
            if (!currentSub.text.isEmpty()) currentSub.text += "\n";
            currentSub.text += line;
        }
    }
    
    if (readingText) {
        m_subtitles.push_back(currentSub);
    }
}

void EditorController::requestWebcamPiP() {
    // To be implemented
}

void EditorController::togglePlayback() {
    m_playbackEngine.togglePlayback();
}

void EditorController::seek(qint64 timeMs) {
    m_playbackEngine.seek(timeMs);
}

void EditorController::syncTime(qint64 mediaTimeMs) {
    m_playbackEngine.syncTime(mediaTimeMs);
}

QString EditorController::projectUuid() const {
    if (m_currentProject) return QString::fromStdString(m_currentProject->uuid);
    return "None";
}

int EditorController::mouseEventCount() const {
    if (m_currentProject) return static_cast<int>(m_currentProject->mouseEvents.size());
    return 0;
}

QString EditorController::projectPath() const { return m_projectPath; }

QString EditorController::videoUrl() const {
    if (m_projectPath.isEmpty()) return "";
    return "file://" + m_projectPath + "/video.mp4";
}

QString EditorController::webcamUrl() const {
    if (m_currentProject) {
        QString path = m_projectPath + "/webcam.mp4";
        if (QFile::exists(path)) {
            return "file://" + path;
        }
    }
    return "";
}

QString EditorController::systemAudioUrl() const {
    if (m_currentProject) {
        QString path = m_projectPath + "/system.m4a";
        if (QFile::exists(path)) {
            return "file://" + path;
        }
    }
    return "";
}

QString EditorController::micAudioUrl() const {
    if (m_currentProject) {
        QString path = m_projectPath + "/mic.m4a";
        if (QFile::exists(path)) {
            return "file://" + path;
        }
    }
    return "";
}

int EditorController::rawVideoWidth() const {
    if (m_currentProject) return m_currentProject->videoWidth;
    return 1920;
}

int EditorController::rawVideoHeight() const {
    if (m_currentProject) return m_currentProject->videoHeight;
    return 1080;
}

int EditorController::videoWidth() const {
    if (!m_currentProject) return 1920;
    int rawW = m_currentProject->videoWidth;
    int rawH = m_currentProject->videoHeight;
    
    int w = rawW;
    switch (m_aspectRatio) {
        case AspectRatio::Ratio9x16:
            w = rawH * 9 / 16;
            break;
        case AspectRatio::Ratio1x1:
            w = std::min(rawW, rawH);
            break;
        case AspectRatio::Ratio16x9:
            w = rawW; // Assumes raw is approx 16:9, but strictly 16:9 is width = width, height = width * 9/16
            break;
        case AspectRatio::RatioOriginal:
        default:
            w = rawW;
            break;
    }
    
    // H.264 hardware encoders require even dimensions
    if (w % 2 != 0) w--;
    return w;
}

int EditorController::videoHeight() const {
    if (!m_currentProject) return 1080;
    int rawW = m_currentProject->videoWidth;
    int rawH = m_currentProject->videoHeight;
    
    int h = rawH;
    switch (m_aspectRatio) {
        case AspectRatio::Ratio9x16:
            h = rawH;
            break;
        case AspectRatio::Ratio1x1:
            h = std::min(rawW, rawH);
            break;
        case AspectRatio::Ratio16x9:
            h = rawW * 9 / 16;
            break;
        case AspectRatio::RatioOriginal:
        default:
            h = rawH;
            break;
    }
    
    // H.264 hardware encoders require even dimensions
    if (h % 2 != 0) h--;
    return h;
}

EditorController::AspectRatio EditorController::aspectRatio() const {
    return m_aspectRatio;
}

void EditorController::loadProject(const QString& path) {
    if (m_autoSaveManager) m_autoSaveManager->stopAndCommit();

    core::logging::Logger::info("Loading project from: " + path.toStdString());
    m_projectPath = path;
    
    QString metaPath = path + "/metadata.json";
    QString autosavePath = path + "/metadata.json.autosave";
    QString fileToLoad = metaPath;

    // Check for autosave file
    if (QFile::exists(autosavePath)) {
        // Compare timestamps
        QFileInfo originalFi(metaPath);
        QFileInfo autosaveFi(autosavePath);
        if (autosaveFi.lastModified() > originalFi.lastModified()) {
            core::logging::Logger::info("Crash detected! Auto-recovering session from autosave...");
            fileToLoad = autosavePath;
        } else {
            // Autosave is older, it's garbage left over from something else
            QFile::remove(autosavePath);
        }
    }

    QFile file(fileToLoad);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString jsonStr = file.readAll();
        file.close();
        auto metadataOpt = project::MetadataSerializer::deserialize(jsonStr);
        if (metadataOpt) {
            m_currentProject = metadataOpt;
            emit projectLoaded();
            m_playbackEngine.loadMetadata(*m_currentProject, videoWidth(), videoHeight());
            parseVttFile(path + "/transcription.vtt");
            emit durationLoaded(totalDuration());
            
            if (m_waveformExtractor) {
                m_waveformExtractor->startExtraction(videoUrl());
            }

            emit projectLoaded();
            
            // Emit property changes to sync QML UI with the loaded metadata
            emit trimStartChanged();
            emit trimEndChanged();
            emit cursorColorChanged();
            emit cursorScaleChanged();
            emit backgroundColorChanged();
            emit aspectRatioChanged();
            emit markersChanged();
            
            core::logging::Logger::info("Editor successfully loaded project metadata.");
        } else {
            core::logging::Logger::error("Failed to deserialize project metadata.");
        }
    } else {
        core::logging::Logger::error("Failed to open metadata file for reading.");
    }
    
    if (m_autoSaveManager) m_autoSaveManager->start(path);
}

void EditorController::setAspectRatio(AspectRatio ratio) {
    if (m_aspectRatio != ratio) {
        m_aspectRatio = ratio;
        emit aspectRatioChanged();
        
        // When aspect ratio changes, we must regenerate the camera keyframes!
        if (m_currentProject) {
            m_playbackEngine.loadMetadata(*m_currentProject, videoWidth(), videoHeight());
        }
    }
}

bool EditorController::isPlaying() const { return m_playbackEngine.isPlaying(); }
qint64 EditorController::currentTime() const { return m_playbackEngine.currentTime(); }
qint64 EditorController::totalDuration() const { return m_playbackEngine.totalDuration(); }
double EditorController::cursorX() const { return m_playbackEngine.cursorX(); }
double EditorController::cursorY() const { return m_playbackEngine.cursorY(); }
int EditorController::cursorState() const { return m_playbackEngine.cursorState(); }
QString EditorController::cursorType() const { return m_playbackEngine.cursorType(); }
double EditorController::cursorWidth() const { return m_playbackEngine.cursorWidth(); }
double EditorController::cursorHeight() const { return m_playbackEngine.cursorHeight(); }
double EditorController::cursorHotspotX() const { return m_playbackEngine.cursorHotspotX(); }
double EditorController::cursorHotspotY() const { return m_playbackEngine.cursorHotspotY(); }
double EditorController::cursorAnchorX() const { return m_playbackEngine.cursorAnchorX(); }
double EditorController::cursorAnchorY() const { return m_playbackEngine.cursorAnchorY(); }
double EditorController::cursorBounceScale() const { return m_playbackEngine.cursorBounceScale(); }
double EditorController::cursorSwayAngle() const { return m_playbackEngine.cursorSwayAngle(); }
double EditorController::cursorZoomScale() const { return m_playbackEngine.cursorZoomScale(); }
qint64 EditorController::timeSinceClick() const { return m_playbackEngine.timeSinceClickMs(); }

double EditorController::cameraX() const { return m_playbackEngine.cameraX(); }
double EditorController::cameraY() const { return m_playbackEngine.cameraY(); }
double EditorController::cameraZoom() const { return m_playbackEngine.cameraZoom(); }
double EditorController::cameraRotation() const { return m_playbackEngine.cameraRotation(); }

QVideoSink* EditorController::videoSink() const { return m_videoSink; }

void EditorController::setVideoSink(QVideoSink* sink) {
    if (m_videoSink != sink) {
        m_videoSink = sink;
        emit videoSinkChanged();
    }
}

QVideoSink* EditorController::webcamSink() const { return m_webcamSink; }

void EditorController::setWebcamSink(QVideoSink* sink) {
    if (m_webcamSink != sink) {
        m_webcamSink = sink;
        emit webcamSinkChanged();
    }
}

void EditorController::saveProject() {
    if (m_currentProject && !m_projectPath.isEmpty()) {
        QString metaPath = m_projectPath + "/metadata.json";
        QString jsonStr = project::MetadataSerializer::serialize(*m_currentProject);
        QFile file(metaPath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            file.write(jsonStr.toUtf8());
            file.close();
            core::logging::Logger::info("Editor saved project metadata.");
        }
    }
}

qint64 EditorController::trimStart() const { return m_currentProject ? m_currentProject->trimStartTimeMs : 0; }
void EditorController::setTrimStart(qint64 timeMs) {
    if (m_currentProject && m_currentProject->trimStartTimeMs != timeMs) {
        m_currentProject->trimStartTimeMs = timeMs;
        emit trimStartChanged();
    }
}

qint64 EditorController::trimEnd() const { return m_currentProject ? m_currentProject->trimEndTimeMs : 0; }
void EditorController::setTrimEnd(qint64 timeMs) {
    if (m_currentProject && m_currentProject->trimEndTimeMs != timeMs) {
        m_currentProject->trimEndTimeMs = timeMs;
        emit trimEndChanged();
    }
}

void EditorController::updateDurationFromMedia(qint64 durationMs) {
    if (durationMs > m_playbackEngine.totalDuration()) {
        m_playbackEngine.setTotalDuration(durationMs);
        // Only override trimEnd if it was either 0 or exactly the old total duration
        if (trimEnd() == 0 || trimEnd() < durationMs) {
            setTrimEnd(durationMs);
        }
        emit durationLoaded(durationMs);
        core::logging::Logger::info("EditorController: Updated duration from media: " + std::to_string(durationMs) + " ms");
    }
}

void EditorController::updateResolutionFromMedia(int width, int height) {
    if (m_currentProject && width > 0 && height > 0) {
        // Only override if the current metadata has the default fallback sizes
        if (m_currentProject->videoWidth == 1920 && m_currentProject->videoHeight == 1080) {
            if (width != 1920 || height != 1080) {
                m_currentProject->videoWidth = width;
                m_currentProject->videoHeight = height;
                core::logging::Logger::info("EditorController: Updated resolution from media: " + std::to_string(width) + "x" + std::to_string(height));
                
                // Recalculate camera keyframes with new resolution
                m_playbackEngine.loadMetadata(*m_currentProject, videoWidth(), videoHeight());
                
                emit projectLoaded();
            }
        }
    }
}

QString EditorController::cursorColor() const { return m_currentProject ? QString::fromStdString(m_currentProject->cursorColor) : "#ff3333"; }
void EditorController::setCursorColor(const QString& color) {
    if (m_currentProject && m_currentProject->cursorColor != color.toStdString()) {
        m_currentProject->cursorColor = color.toStdString();
        emit cursorColorChanged();
    }
}

double EditorController::cursorScale() const { return m_currentProject ? m_currentProject->cursorScale : 1.0; }
void EditorController::setCursorScale(double scale) {
    if (m_currentProject && m_currentProject->cursorScale != scale) {
        m_currentProject->cursorScale = scale;
        emit cursorScaleChanged();
    }
}

QString EditorController::backgroundColor() const { return m_currentProject ? QString::fromStdString(m_currentProject->backgroundColor) : "#000000"; }
void EditorController::setBackgroundColor(const QString& color) {
    if (m_currentProject && m_currentProject->backgroundColor != color.toStdString()) {
        m_currentProject->backgroundColor = color.toStdString();
        emit backgroundColorChanged();
    }
}

QVariantList EditorController::markers() const {
    QVariantList list;
    if (m_currentProject) {
        for (const auto& m : m_currentProject->markers) {
            QVariantMap map;
            map["timestampMs"] = static_cast<qulonglong>(m.timestampMs);
            map["color"] = QString::fromStdString(m.color);
            map["note"] = QString::fromStdString(m.note);
            list.append(map);
        }
    }
    return list;
}

void EditorController::addMarker(qint64 timestampMs, const QString& note, const QString& color) {
    if (m_currentProject) {
        project::ProjectMetadata::Marker m;
        m.timestampMs = timestampMs;
        m.color = color.toStdString();
        m.note = note.toStdString();
        m_currentProject->markers.push_back(m);
        emit markersChanged();
        saveProject();
    }
}

void EditorController::removeMarker(int index) {
    if (m_currentProject && index >= 0 && index < static_cast<int>(m_currentProject->markers.size())) {
        m_currentProject->markers.erase(m_currentProject->markers.begin() + index);
        emit markersChanged();
        saveProject();
    }
}

void EditorController::updateMarkerNote(int index, const QString& note) {
    if (m_currentProject && index >= 0 && index < static_cast<int>(m_currentProject->markers.size())) {
        m_currentProject->markers[index].note = note.toStdString();
        emit markersChanged();
        saveProject();
    }
}

void EditorController::saveAsPreset(const QString& presetName) {
    if (m_currentProject) {
        project::PresetManager pm;
        project::Preset p;
        p.name = presetName;
        p.style.cursorColor = m_currentProject->cursorColor;
        p.style.cursorScale = m_currentProject->cursorScale;
        p.style.backgroundColor = m_currentProject->backgroundColor;
        pm.savePreset(p);
    }
}

void EditorController::applyPreset(const QString& presetName) {
    if (m_currentProject) {
        project::PresetManager pm;
        project::Preset p;
        if (pm.loadPreset(presetName, p)) {
            pm.applyStyleToProject(*m_currentProject, p);
            emit cursorColorChanged();
            emit cursorScaleChanged();
            emit backgroundColorChanged();
            saveProject();
        }
    }
}

QList<QString> EditorController::getAvailablePresets() const {
    project::PresetManager pm;
    return pm.getAvailablePresets();
}

QStringList EditorController::getAvailableWallpapers() const {
    return QStringList{
        "sonoma-dark.jpg",
        "sonoma-light.jpg",
        "sonoma-evening.jpg",
        "sonoma-clouds.jpg",
        "sonoma-horizon.jpg",
        "sequoia-blue.jpg",
        "sequoia-blue-orange.jpg",
        "tahoe-dark.jpg",
        "tahoe-light.jpg",
        "ventura-dark.jpg",
        "ventura.jpg",
        "ipad-17-dark.jpg",
        "ipad-17-light.jpg",
        "bluerays.jpeg",
        "lemonade.jpeg",
        "iridescent-9.jpg",
        "glassmorphism-3.jpg",
        "glassmorphism-4.jpg",
        "midnight-8.jpg",
        "mountaintrees.jpg",
        "farmvalley.jpg",
        "cityscape.jpg",
        "cherrypop.jpg",
        "energy-17.jpg",
        "energy-19.jpg",
        "levels.jpg",
        "luisdelrio.jpg",
        "wallpaper1.jpg",
        "wallpaper2.jpg",
        "wallpaper3.jpg",
        "wallpaper4.jpg",
        "wallpaper7.jpg",
        "wallpaper9.jpg",
        "wallpaper10.jpg",
        "wallpaper11.jpg",
        "wallpaper12.jpg",
        "wallpaper13.jpg",
        "wallpaper15.jpg"
    };
}

} // namespace ssa::editor
