#include "export/ExportEngine.h"
#include "project/MetadataSerializer.h"
#include "core/logging/Logger.h"
#include "project/PresetManager.h"
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QProcess>
#include <QFileInfo>
#include <QFile>
#include <fmt/core.h>
#include <QDir>
#include <QCoreApplication>
#include <unordered_map>

#include <QSvgRenderer>

static QString getAppDataDirForExport() {
    QDir current(QDir::currentPath());
    if (current.exists("cursors") || current.exists("CMakeLists.txt")) {
        return current.absolutePath();
    }
    QDir parentDir = current;
    while (parentDir.cdUp()) {
        if (parentDir.exists("cursors") || parentDir.exists("CMakeLists.txt")) {
            return parentDir.absolutePath();
        }
    }
    return QDir::currentPath();
}

static QImage getCursorImage(const std::string& typeInput) {
    std::string type = typeInput.empty() ? "arrow" : typeInput;
    static std::unordered_map<std::string, QImage> cache;
    if (cache.find(type) != cache.end()) {
        return cache[type];
    }
    
    QString baseDir = getAppDataDirForExport();
    
    // Map cursor type to PNG filename in cursors/
    static const std::unordered_map<std::string, QString> pngMap = {
        {"arrow", "arrow.png"},
        {"pointer", "pointingHand.png"},
        {"pointingHand", "pointingHand.png"},
        {"iBeam", "iBeam.png"},
        {"iBeamCursorForVerticalLayout", "iBeamCursorForVerticalLayout.png"},
        {"closedHand", "closedHand.png"},
        {"openHand", "openHand.png"},
        {"crosshair", "crosshair.png"},
        {"busyButClickable", "busyButClickable.png"},
        {"contextualMenu", "contextualMenu.png"},
        {"dragCopy", "dragCopy.png"},
        {"dragLink", "dragLink.png"},
        {"operationNotAllowed", "operationNotAllowed.png"},
        {"help", "help.png"},
        {"zoomIn", "zoomIn.png"},
        {"zoomOut", "zoomOut.png"},
        {"resizeLeftRight", "resizeLeftRight.png"},
        {"resizeUpDown", "resizeUpDown.png"},
        {"resizeDown", "resizeDown.png"},
        {"resizeUp", "resizeUp.png"},
        {"resizeLeft", "resizeLeft.png"},
        {"resizeRight", "resizeRight.png"}
    };
    
    QString pngFile;
    auto it = pngMap.find(type);
    if (it != pngMap.end()) {
        pngFile = it->second;
    } else {
        pngFile = QString::fromStdString(type) + ".png";
    }
    
    QString pngPath = baseDir + "/cursors/" + pngFile;
    if (!QFile::exists(pngPath)) {
        pngPath = baseDir + "/cursors/arrow.png";
    }
    
    QImage img(pngPath);
    if (img.isNull()) {
        ssa::core::logging::Logger::error("Failed to load cursor image: " + pngPath.toStdString());
    }
    
    cache[type] = img;
    return cache[type];
}

static QPoint getCursorHotspot(const std::string& type) {
    static std::unordered_map<std::string, QPoint> hotspots = {
        {"arrow", QPoint(5, 5)},
        {"pointingHand", QPoint(13, 8)},
        {"iBeam", QPoint(12, 11)},
        {"resizeLeftRight", QPoint(15, 12)},
        {"resizeUp", QPoint(12, 12)},
        {"resizeDown", QPoint(12, 12)},
        {"crosshair", QPoint(12, 12)},
        {"dragCopy", QPoint(5, 5)}
    };
    auto it = hotspots.find(type);
    if (it != hotspots.end()) return it->second;
    return QPoint(5, 5); // Default to arrow hotspot
}

namespace ssa::export_engine {

ExportEngine::ExportEngine(QObject* parent)
    : QObject(parent) {
}

ExportEngine::~ExportEngine() {
    cancelExport();
    if (m_exportThread.joinable()) {
        m_exportThread.join();
    }
}

void ExportEngine::startExport(const QString& projectPath, const QString& presetName, const QString& resolution, const QString& outputPath) {
    if (m_isExporting) return;
    if (m_exportThread.joinable()) {
        m_exportThread.join();
    }
    
    m_projectPath = projectPath;
    m_outputPath = outputPath;
    
    QFile metaFile(projectPath + "/metadata.json");
    if (!metaFile.open(QIODevice::ReadOnly)) {
        core::logging::Logger::error("ExportEngine: Failed to open metadata.json");
        return;
    }
    QString jsonStr = metaFile.readAll();
    metaFile.close();
    
    auto metadataOpt = project::MetadataSerializer::deserialize(jsonStr);
    if (!metadataOpt) {
        core::logging::Logger::error("ExportEngine: Failed to deserialize metadata");
        return;
    }
    m_metadata = *metadataOpt;
    
    m_playbackEngine = std::make_unique<editor::PlaybackEngine>();
    m_playbackEngine->loadMetadata(m_metadata, -1, -1);
    
    project::PresetManager pm;
    project::Preset preset;
    pm.loadPreset(presetName, preset);

    m_encoder = createMediaEncoder();
    m_decoder = createMediaDecoder();
    
    int width = m_metadata.videoWidth;
    int height = m_metadata.videoHeight;
    
    if (resolution == "4K") { width = 3840; height = 2160; }
    else if (resolution == "1440p") { width = 2560; height = 1440; }
    else if (resolution == "1080p") { width = 1920; height = 1080; }
    else if (resolution == "720p") { width = 1280; height = 720; }
    
    if (preset.exportCfg.aspectRatio == "9:16") {
        std::swap(width, height);
    } else if (preset.exportCfg.aspectRatio == "1:1") {
        width = height = std::min(width, height);
    }
    
    EncoderConfig config;
    config.codec = preset.exportCfg.codec;
    config.quality = preset.exportCfg.quality;
    config.hardwareAccel = preset.exportCfg.hardwareAccel;
    
    if (!m_encoder->initialize(outputPath.toStdString(), width, height, m_fps, config)) {
        core::logging::Logger::error("Failed to initialize Media Encoder");
        finishExport(false);
        return;
    }
    
    QString videoUrl = m_projectPath + "/video.mp4";
    if (videoUrl.startsWith("file://")) videoUrl = videoUrl.mid(7); // Remove scheme
    
    if (!m_decoder->initialize(videoUrl.toStdString())) {
        core::logging::Logger::error("Failed to initialize Media Decoder with source video");
        finishExport(false);
        return;
    }
    
    QString webcamUrl = m_projectPath + "/webcam.mp4";
    if (QFile::exists(webcamUrl)) {
        if (webcamUrl.startsWith("file://")) webcamUrl = webcamUrl.mid(7);
        m_webcamDecoder = createMediaDecoder();
        if (!m_webcamDecoder->initialize(webcamUrl.toStdString())) {
            core::logging::Logger::error("Failed to initialize Webcam Decoder, proceeding without webcam");
            m_webcamDecoder.reset(); // non-fatal
        }
    } else {
        m_webcamDecoder.reset();
    }
    
    m_isExporting = true;
    m_cancelRequested = false;
    m_progress = 0.0;
    m_totalDurationMs = m_playbackEngine->totalDuration();
    
    emit isExportingChanged(true);
    emit progressChanged(0.0);
    
    core::logging::Logger::info("Starting offline video export to: " + outputPath.toStdString());
    
    m_exportThread = std::thread(&ExportEngine::runExportLoop, this);
}

void ExportEngine::cancelExport() {
    if (!m_isExporting) return;
    
    m_cancelRequested = true;
    core::logging::Logger::info("Canceling video export...");
}

    void ExportEngine::runExportLoop() {
    int width = m_metadata.videoWidth;
    int height = m_metadata.videoHeight;
    int rawWidth = m_metadata.videoWidth;
    int rawHeight = m_metadata.videoHeight;
    
    qint64 currentTimeMs = m_metadata.trimStartTimeMs;
    qint64 endTimeMs = m_metadata.trimEndTimeMs;
    if (endTimeMs <= 0 || endTimeMs > m_totalDurationMs) {
        endTimeMs = m_totalDurationMs;
    }
    
    qint64 stepMs = 1000 / m_fps;
    
    while (currentTimeMs <= endTimeMs) {
        if (m_cancelRequested) {
            break;
        }
        
        QImage frameImage = m_decoder->getFrameAtTime(currentTimeMs);
        if (frameImage.isNull()) {
            // Fill with black if extraction failed
            frameImage = QImage(rawWidth, rawHeight, QImage::Format_ARGB32);
            frameImage.fill(Qt::black);
        }
        
        // Temporal Supersampling (Motion Blur)
        const int SUB_FRAMES = 8;
        std::vector<float> accum(width * height * 4, 0.0f);
        
        // 180-degree shutter simulation
        float shutterMs = static_cast<float>(stepMs) / 2.0f; 
        float subStepMs = shutterMs / SUB_FRAMES;
        float startSubTimeMs = currentTimeMs - (shutterMs / 2.0f);
        
        QImage subImage(width, height, QImage::Format_ARGB32);
        
        for (int i = 0; i < SUB_FRAMES; ++i) {
            qint64 subTimeMs = static_cast<qint64>(startSubTimeMs + i * subStepMs);
            if (subTimeMs < 0) subTimeMs = 0;
            
            // Retrieve pure mathematical state directly from the camera engine for this specific offline time
            auto camState = m_playbackEngine->cameraEngine().evaluateAtTime(subTimeMs);
            double cx = camState.x;
            double cy = camState.y;
            double zoom = camState.zoom;
            
            int cursorState = 0;
            double cursorX = 0;
            double cursorY = 0;
            std::string cursorType;
            qint64 lastClickTimeMs = 0;
            double cursorWidth = 28.0;
            double cursorHeight = 40.0;
            double anchorX = 0.34;
            double anchorY = 0.24;
            double bounceScale = 1.0;
            double swayAngle = 0.0;
            double zoomScale = 1.0;
            m_playbackEngine->evaluateCursorStateOffline(subTimeMs, cursorX, cursorY, cursorState, cursorType, lastClickTimeMs, cursorWidth, cursorHeight, anchorX, anchorY, bounceScale, swayAngle, zoomScale);
            
            // Draw the composition background (Solid color or Recordly Wallpaper image)
            std::string bgStr = m_metadata.backgroundColor;
            QString bg = QString::fromStdString(bgStr);
            bool filledWp = false;
            if (!bg.startsWith("#") && !bg.isEmpty()) {
                QString wallpaperPath = bg;
                if (!QFile::exists(wallpaperPath)) {
                    QString p1 = QDir::currentPath() + "/wallpapers/" + bg;
                    QString p2 = QCoreApplication::applicationDirPath() + "/wallpapers/" + bg;
                    QString p3 = QDir(m_projectPath).absoluteFilePath("../../wallpapers/" + bg);
                    QString p4 = "/Users/maitraprajapati/Desktop/ssA31/wallpapers/" + bg;
                    
                    if (QFile::exists(p1)) wallpaperPath = p1;
                    else if (QFile::exists(p2)) wallpaperPath = p2;
                    else if (QFile::exists(p3)) wallpaperPath = p3;
                    else if (QFile::exists(p4)) wallpaperPath = p4;
                }
                
                QImage wp(wallpaperPath);
                if (!wp.isNull()) {
                    QPainter wpPainter(&subImage);
                    wpPainter.setRenderHint(QPainter::SmoothPixmapTransform);
                    wpPainter.drawImage(QRect(0, 0, width, height), wp.scaled(width, height, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
                    wpPainter.end();
                    filledWp = true;
                }
            }
            if (!filledWp) {
                subImage.fill(QColor(bg.startsWith("#") ? bg : "#0f172a"));
            }
            
            QPainter painter(&subImage);
            painter.setRenderHint(QPainter::Antialiasing);
            painter.setRenderHint(QPainter::SmoothPixmapTransform);
            
            // Calculate aspect ratio scale to fit the target resolution with 0.88 inset framing matching Live Preview
            double logicalWidth = m_metadata.videoWidth;
            double logicalHeight = m_metadata.videoHeight;
            double videoPaddingScale = 0.88; // 12% padding around video to frame Recordly wallpaper background
            double globalScale = std::min((double)width / logicalWidth, (double)height / logicalHeight) * videoPaddingScale;
            
            painter.translate(width / 2.0, height / 2.0);
            painter.scale(globalScale, globalScale); // Scale up logical space to preset resolution with video padding
            painter.scale(zoom, zoom);
            painter.rotate(camState.rotation);
            painter.translate(-cx, -cy);
            
            painter.drawImage(QRect(0, 0, rawWidth, rawHeight), frameImage);
            
            // Draw cursor
            double baseCursorScale = m_metadata.cursorScale;
            QImage cursorImg = getCursorImage(cursorType);
            
            // Draw click vector animations (Recordly Ripple, Spotlight, Echo)
            if (lastClickTimeMs > 0 && subTimeMs >= lastClickTimeMs) {
                qint64 timeSinceClick = subTimeMs - lastClickTimeMs;
                const double durationMs = 350.0;
                if (timeSinceClick <= durationMs) {
                    double clickProg = 1.0 - (timeSinceClick / durationMs); // 1.0 down to 0.0
                    double eased = 1.0 - std::pow(clickProg, 3.0); // 0.0 expanding to 1.0
                    double fade = std::pow(clickProg, 3.0);
                    
                    double scaledH = cursorHeight * baseCursorScale * zoomScale;
                    double maxRippleRadius = std::max(5.0, eased * scaledH * 1.95);
                    double opacity = std::max(0.0, std::min(1.0, fade * 0.85));
                    
                    QColor color = QColor(QString::fromStdString(m_metadata.cursorColor));
                    if (!color.isValid()) color = QColor("#ff3366");
                    
                    // 1. Primary Expanding Ripple Ring
                    QColor strokeColor = color;
                    strokeColor.setAlphaF(opacity);
                    painter.setBrush(Qt::NoBrush);
                    QPen ripplePen(strokeColor);
                    ripplePen.setWidthF(std::max(1.5, 3.0 * fade));
                    painter.setPen(ripplePen);
                    painter.drawEllipse(QPointF(cursorX, cursorY), maxRippleRadius, maxRippleRadius);
                    
                    // 2. Spotlight Concentric Inner Ring
                    double innerRadius = maxRippleRadius * 0.62;
                    QColor spotColor = color;
                    spotColor.setAlphaF(opacity * 0.5);
                    QPen spotPen(spotColor);
                    spotPen.setWidthF(1.5);
                    painter.setPen(spotPen);
                    painter.drawEllipse(QPointF(cursorX, cursorY), innerRadius, innerRadius);
                    
                    // 3. Central Click Accent Dot
                    if (timeSinceClick <= 200) {
                        double dotProg = 1.0 - (timeSinceClick / 200.0);
                        double dotRadius = std::max(2.0, scaledH * 0.35 * dotProg);
                        QColor dotColor = color;
                        dotColor.setAlphaF(opacity * 0.9);
                        painter.setBrush(dotColor);
                        painter.setPen(Qt::NoPen);
                        painter.drawEllipse(QPointF(cursorX, cursorY), dotRadius, dotRadius);
                    }
                }
            }
            
            // Apply cursor transformation (Position, Scale, Bounce, Sway Rotation, Anchor Offset)
            painter.save();
            painter.translate(cursorX, cursorY);
            
            if (std::abs(swayAngle) > 0.01) {
                painter.rotate(swayAngle);
            }
            
            double styleMultiplier = editor::PlaybackEngine::getCursorStyleMultiplier(cursorType);
            double finalScale = baseCursorScale * styleMultiplier * bounceScale * zoomScale;
            painter.scale(finalScale, finalScale);
            
            if (!cursorImg.isNull()) {
                double drawX = -anchorX * cursorWidth;
                double drawY = -anchorY * cursorHeight;
                painter.drawImage(QRectF(drawX, drawY, cursorWidth, cursorHeight), cursorImg);
            } else {
                painter.setBrush(QColor(QString::fromStdString(m_metadata.cursorColor)));
                QPen pen(Qt::black);
                pen.setWidth(2);
                painter.setPen(pen);
                painter.drawEllipse(QPointF(0, 0), 10, 10);
            }
            painter.restore();
            painter.end();
            
            // Accumulate
            const uint8_t* bits = subImage.constBits();
            for (size_t p = 0; p < accum.size(); ++p) {
                accum[p] += bits[p];
            }
        }
        
        // Finalize blended frame
        QImage finalImage(width, height, QImage::Format_ARGB32);
        uint8_t* finalBits = finalImage.bits();
        for (size_t p = 0; p < accum.size(); ++p) {
            finalBits[p] = static_cast<uint8_t>(std::clamp(accum[p] / SUB_FRAMES, 0.0f, 255.0f));
        }
        
        // Composite webcam PiP over the final motion blurred image
        if (m_webcamDecoder) {
            QImage webcamImage = m_webcamDecoder->getFrameAtTime(currentTimeMs);
            if (!webcamImage.isNull()) {
                QPainter pipPainter(&finalImage);
                pipPainter.setRenderHint(QPainter::Antialiasing);
                pipPainter.setRenderHint(QPainter::SmoothPixmapTransform);
                
                int pipSize = std::min(width, height) / 4;
                QRect pipRect(width - pipSize - 40, height - pipSize - 40, pipSize, pipSize);
                
                // Draw circular mask using clipping
                QPainterPath path;
                path.addEllipse(pipRect);
                pipPainter.setClipPath(path);
                
                // Draw the webcam image stretched to fit the circle
                pipPainter.drawImage(pipRect, webcamImage);
                pipPainter.end();
                
                // Draw border
                QPainter borderPainter(&finalImage);
                borderPainter.setRenderHint(QPainter::Antialiasing);
                QPen pipPen(Qt::white, 3);
                borderPainter.setPen(pipPen);
                borderPainter.setBrush(Qt::NoBrush);
                borderPainter.drawEllipse(pipRect);
                borderPainter.end();
            }
        }
        
        // Notify progress
        qint64 relativeTimeMs = currentTimeMs - m_metadata.trimStartTimeMs;
        uint64_t pts = relativeTimeMs * 1000;
        
        if (!m_encoder->appendFrame(finalImage, pts)) {
            core::logging::Logger::error("Failed to append offline frame to encoder at time " + std::to_string(currentTimeMs));
            m_cancelRequested = true;
            break;
        }
        
        currentTimeMs += stepMs;
        
        if (endTimeMs > m_metadata.trimStartTimeMs) {
            double progress = static_cast<double>(currentTimeMs - m_metadata.trimStartTimeMs) / 
                              static_cast<double>(endTimeMs - m_metadata.trimStartTimeMs);
            if (progress > 1.0) progress = 1.0;
            m_progress = progress;
            
            core::logging::Logger::info("Export Progress: " + std::to_string(static_cast<int>(progress * 100.0)) + "%");
            
            // Emit via QMetaObject since we are in a background thread
            QMetaObject::invokeMethod(this, [this, progress]() {
                emit progressChanged(progress);
            });
        }
    }
    
    QMetaObject::invokeMethod(this, [this]() {
        finishExport(!m_cancelRequested);
    });
}

void ExportEngine::finishExport(bool success) {
    if (m_encoder) {
        m_encoder->finish();
        m_encoder.reset();
    }
    if (m_decoder) {
        m_decoder.reset();
    }
    
    if (success) {
        QString sysM4a = m_projectPath + "/system.m4a";
        QString sysWav = m_projectPath + "/system_audio.wav";
        QString micM4a = m_projectPath + "/mic.m4a";
        QString micWav = m_projectPath + "/mic_audio.wav";
        QString origVideo = m_projectPath + "/video.mp4";
        
        if (sysM4a.startsWith("file://")) sysM4a = sysM4a.mid(7);
        if (sysWav.startsWith("file://")) sysWav = sysWav.mid(7);
        if (micM4a.startsWith("file://")) micM4a = micM4a.mid(7);
        if (micWav.startsWith("file://")) micWav = micWav.mid(7);
        if (origVideo.startsWith("file://")) origVideo = origVideo.mid(7);
        
        QString sysAudioPath = QFile::exists(sysM4a) ? sysM4a : (QFile::exists(sysWav) ? sysWav : "");
        QString micAudioPath = QFile::exists(micM4a) ? micM4a : (QFile::exists(micWav) ? micWav : "");
        
        bool hasSys = !sysAudioPath.isEmpty();
        bool hasMic = !micAudioPath.isEmpty();
        bool hasOrigVideo = QFile::exists(origVideo);
        
        if (hasSys || hasMic || hasOrigVideo) {
            core::logging::Logger::info("Muxing audio into exported MP4 via FFmpeg...");
            QString tempOutput = m_outputPath + ".tmp.mp4";
            
            double trimStartSec = m_metadata.trimStartTimeMs / 1000.0;
            double trimEndSec = (m_metadata.trimEndTimeMs > 0) ? (m_metadata.trimEndTimeMs / 1000.0) : (m_totalDurationMs / 1000.0);
            double durationSec = (trimEndSec > trimStartSec) ? (trimEndSec - trimStartSec) : (m_totalDurationMs / 1000.0);
            
            QStringList args;
            args << "-y";
            args << "-i" << m_outputPath; // Video input 0
            
            if (hasSys && hasMic) {
                args << "-ss" << QString::number(trimStartSec, 'f', 3)
                     << "-t" << QString::number(durationSec, 'f', 3)
                     << "-i" << sysAudioPath; // Input 1
                     
                args << "-ss" << QString::number(trimStartSec, 'f', 3)
                     << "-t" << QString::number(durationSec, 'f', 3)
                     << "-i" << micAudioPath; // Input 2
                     
                args << "-filter_complex" << "[1:a][2:a]amix=inputs=2:duration=longest[a]"
                     << "-map" << "0:v" << "-map" << "[a]";
            } else if (hasSys) {
                args << "-ss" << QString::number(trimStartSec, 'f', 3)
                     << "-t" << QString::number(durationSec, 'f', 3)
                     << "-i" << sysAudioPath;
                args << "-map" << "0:v" << "-map" << "1:a";
            } else if (hasMic) {
                args << "-ss" << QString::number(trimStartSec, 'f', 3)
                     << "-t" << QString::number(durationSec, 'f', 3)
                     << "-i" << micAudioPath;
                args << "-map" << "0:v" << "-map" << "1:a";
            } else if (hasOrigVideo) {
                // Mux audio track directly from original recorded video.mp4
                args << "-ss" << QString::number(trimStartSec, 'f', 3)
                     << "-t" << QString::number(durationSec, 'f', 3)
                     << "-i" << origVideo;
                args << "-map" << "0:v" << "-map" << "1:a?";
            }
            
            args << "-c:v" << "copy" << "-c:a" << "aac" << "-b:a" << "192k" << tempOutput;
            
            QProcess ffmpeg;
            ffmpeg.start("ffmpeg", args);
            if (ffmpeg.waitForFinished(-1) && ffmpeg.exitCode() == 0) {
                QFile::remove(m_outputPath);
                QFile::rename(tempOutput, m_outputPath);
                core::logging::Logger::info("Audio muxed successfully into final export.");
            } else {
                core::logging::Logger::error("FFmpeg audio muxing failed: " + QString(ffmpeg.readAllStandardError()).toStdString());
                QFile::remove(tempOutput);
            }
        }
    }
    
    m_isExporting = false;
    emit isExportingChanged(false);
    emit exportFinished(success, m_outputPath);
    
    if (m_exportThread.joinable()) {
        m_exportThread.join();
    }
}

} // namespace ssa::export_engine
