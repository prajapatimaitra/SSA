#include "CaptureController.h"
#include "core/logging/Logger.h"
#include "ai/MockTranscriptionModel.h"
#include "ai/AIEngine.h"
#include "library/LibraryManager.h"
#include "project/PresetManager.h"
#include <QMediaCaptureSession>
#include <QAudioInput>
#include <QMediaRecorder>
#include <QMediaFormat>
#include <QUrl>
#include <fmt/core.h>
#include <QDir>

namespace ssa::capture {

CaptureController::CaptureController(std::unique_ptr<IPlatformCapture> captureEngine, 
                                   std::unique_ptr<IScreenCapturer> screenCapturer,
                                   std::unique_ptr<input::IInputTracker> inputTracker,
                                   QObject* parent)
    : QObject(parent), m_captureEngine(std::move(captureEngine)), m_screenCapturer(std::move(screenCapturer)), m_inputTracker(std::move(inputTracker)) {
    
    if (m_captureEngine) {
        m_captureEngine->setFrameCallback([this](const media::VideoFrame& frame) {
            std::lock_guard<std::mutex> lock(m_eventsMutex);
            if (m_firstFrameTimestamp == 0) {
                auto now = std::chrono::high_resolution_clock::now();
                m_firstFrameTimestamp = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();
            }
            // Store dimensions to save into metadata
            m_lastVideoWidth = frame.width;
            m_lastVideoHeight = frame.height;
        });
    }

    if (m_inputTracker) {
        m_inputTracker->setEventCallback([this](const input::MouseEvent& ev) {
            std::lock_guard<std::mutex> lock(m_eventsMutex);
            m_recordedEvents.push_back(ev);
            
            if (ev.type == input::MouseEventType::Down || ev.type == input::MouseEventType::Up) {
                std::string action = (ev.type == input::MouseEventType::Down) ? "Down" : "Up";
                std::string btn = (ev.button == input::MouseButton::Left) ? "Left" : 
                                  (ev.button == input::MouseButton::Right) ? "Right" : "Middle";
                core::logging::Logger::info("Mouse Click " + action + " (" + btn + ") at X: " + std::to_string(ev.x) + ", Y: " + std::to_string(ev.y) + " (Timestamp: " + std::to_string(ev.timestamp) + " us)");
            }
        });
    }
}

void CaptureController::startCapture(const QString& presetName, const QString& displayId, const QByteArray& cameraId, int fps, const QString& resolution, int audioSource, int cropX, int cropY, int cropW, int cropH) {
    m_isStopping = false;
    m_isRecording = true;
    std::string did = displayId.toStdString();
    
    // Create new project bundle path
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    m_projectPath = (m_appDataDir + QString("/.recordings/Recording_%1.ssa").arg(ms)).toStdString();
    
    // Actually create the directory on disk so the video encoder can write to it
    m_projectManager.createNewProject(m_projectPath, 1920, 1080); // Default resolution, updated later
    
    std::string videoOutputPath = m_projectPath + "/video.mp4";
    std::string systemAudioPath = m_projectPath + "/system.m4a";
    std::string micAudioPath = m_projectPath + "/mic.m4a";

    core::logging::Logger::info("Starting capture on display: " + did);
    
    {
        std::lock_guard<std::mutex> lock(m_eventsMutex);
        m_recordedEvents.clear();
        m_firstFrameTimestamp = 0;
    }
    
    // Load Preset for configuration
    project::PresetManager pm;
    project::Preset preset;
    pm.loadPreset(presetName, preset);
    
    // Start Webcam Capture
    m_selectedWebcamId = cameraId;
    if (!m_selectedWebcamId.isEmpty()) {
        if (!m_webcamRecorder) {
            m_webcamRecorder = std::make_unique<WebcamRecorder>(this);
        }
        QString webcamOutputPath = QString::fromStdString(m_projectPath) + "/webcam.mp4";
        m_webcamRecorder->startRecording(m_selectedWebcamId, webcamOutputPath);
    }
    
    // Start Mic Capture if audioSource is 0 (Mic Only) or 2 (Both)
    if (audioSource == 0 || audioSource == 2) {
        if (!m_micCaptureSession) m_micCaptureSession = std::make_unique<QMediaCaptureSession>();
        if (!m_micAudioInput) m_micAudioInput = std::make_unique<QAudioInput>();
        if (!m_micRecorder) m_micRecorder = std::make_unique<QMediaRecorder>();
        
        m_micCaptureSession->setAudioInput(m_micAudioInput.get());
        m_micCaptureSession->setRecorder(m_micRecorder.get());
        m_micRecorder->setOutputLocation(QUrl::fromLocalFile(QString::fromStdString(micAudioPath)));
        
        QMediaFormat format(QMediaFormat::MPEG4);
        format.setAudioCodec(QMediaFormat::AudioCodec::AAC);
        m_micRecorder->setMediaFormat(format);
        m_micRecorder->record();
    }
    
    if (m_captureEngine) {
        CaptureConfig config;
        config.displayId = did;
        config.outputFilePath = videoOutputPath;
        config.fps = fps;
        
        // Setup Region
        config.isCustomRegion = preset.recording.isCustomRegion;
        config.regionX = preset.recording.regionX;
        config.regionY = preset.recording.regionY;
        config.regionWidth = preset.recording.regionWidth;
        config.regionHeight = preset.recording.regionHeight;
        config.captureResolution = resolution.toStdString();
        
        // System Audio if audioSource is 1 (System Only) or 2 (Both)
        if (audioSource == 1 || audioSource == 2) {
            config.systemAudioPath = systemAudioPath;
        } else {
            config.systemAudioPath = "";
        }
        
        config.regionX = cropX;
        config.regionY = cropY;
        config.regionWidth = cropW;
        config.regionHeight = cropH;
        config.isCustomRegion = (cropW > 0 && cropH > 0);
        
        m_captureEngine->startCapture(config);
    }
    
    if (m_inputTracker) {
        m_inputTracker->startTracking();
    }
    
    emit recordingStarted();
}

void CaptureController::stopCapture() {
    if (m_isStopping) return;
    m_isStopping = true;
    m_isRecording = false;
    
    if (m_webcamRecorder) {
        m_webcamRecorder->stopRecording();
    }
    
    if (m_micRecorder && m_micRecorder->recorderState() == QMediaRecorder::RecordingState) {
        m_micRecorder->stop();
    }
    
    if (m_inputTracker) {
        m_inputTracker->stopTracking();
    }
    if (m_captureEngine) {
        core::logging::Logger::info("Requesting stop capture.");
        m_captureEngine->stopCapture();
    }
    
    project::ProjectMetadata meta;
    auto currentMetaOpt = m_projectManager.loadMetadata();
    if (currentMetaOpt) {
        auto meta = *currentMetaOpt;
        {
            std::lock_guard<std::mutex> lock(m_eventsMutex);
            meta.mouseEvents = m_recordedEvents;
            meta.videoWidth = m_lastVideoWidth;
            meta.videoHeight = m_lastVideoHeight;
            if (!m_recordedEvents.empty()) {
                if (m_firstFrameTimestamp == 0 || m_firstFrameTimestamp > m_recordedEvents.front().timestamp) {
                    m_firstFrameTimestamp = m_recordedEvents.front().timestamp;
                }
            }
            meta.recordingStartTimestamp = m_firstFrameTimestamp;
        }
        m_projectManager.saveMetadata(meta);
    }
    
    // Emit captureFinished immediately so the Editor loads instantly
    QString finalPath = QString::fromStdString(m_projectPath);
    emit captureFinished(finalPath);
    
    if (m_libraryManager) {
        library::MediaItem item;
        item.type = library::MediaType::Recording;
        item.path = finalPath;
        item.timestamp = QFileInfo(finalPath).birthTime().toMSecsSinceEpoch();
        m_libraryManager->addMediaItem(item);
    }
    
    if (!m_aiEngine) {
        m_aiEngine = std::make_unique<ai::AIEngine>();
        m_aiEngine->addModel(std::make_unique<ai::MockTranscriptionModel>());
        connect(m_aiEngine.get(), &ai::AIEngine::processingFinished, this, [this](const QString& path) {
            QString vttPath = path + "/transcription.vtt";
            emit subtitlesReady(vttPath);
        });
    }
    m_aiEngine->runProcessing(finalPath);
    
    emit recordingStopped();
}

void CaptureController::captureScreenshot() {
    if (!m_screenCapturer) {
        core::logging::Logger::error("No Screen Capturer configured.");
        return;
    }
    
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    
    QString dirPath;
    if (m_isRecording && !m_projectPath.empty()) {
        dirPath = QString::fromStdString(m_projectPath) + "/screenshots";
    } else {
        dirPath = m_appDataDir + "/.screenshots";
    }
    QDir().mkpath(dirPath);
    
    QString imagePath = QString("%1/Screenshot_%2.png").arg(dirPath).arg(ms);
    
    m_screenCapturer->captureScreenshot("0", imagePath, [this, imagePath](bool success) {
        if (success) {
            core::logging::Logger::info("Screenshot successfully captured: " + imagePath.toStdString());
            
            if (m_libraryManager) {
                library::MediaItem item;
                item.type = library::MediaType::Screenshot;
                item.path = imagePath;
                item.thumbnailPath = imagePath;
                item.timestamp = QFileInfo(imagePath).birthTime().toMSecsSinceEpoch();
                m_libraryManager->addMediaItem(item);
            }
            
            emit screenshotCaptured(imagePath);
        } else {
            core::logging::Logger::error("Failed to capture screenshot.");
        }
    });
}

void CaptureController::importExternalMedia(const QString& mediaPath) {
    QString cleanPath = mediaPath;
    if (cleanPath.startsWith("file://")) {
        cleanPath = cleanPath.mid(7); // Remove file://
    }
    
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    
    QString bundlePath = QString("%1/.recordings/Import_%2.ssa").arg(m_appDataDir).arg(ms);
    
    if (m_projectManager.createProjectFromExternalMedia(cleanPath.toStdString(), bundlePath.toStdString())) {
        core::logging::Logger::info("Successfully imported external media to project: " + bundlePath.toStdString());
        
        if (m_libraryManager) {
            library::MediaItem item;
            item.type = library::MediaType::Recording;
            item.path = bundlePath;
            item.thumbnailPath = "";
            item.timestamp = ms;
            m_libraryManager->addMediaItem(item);
        }
        
        // Trick the UI into thinking a recording just finished, which auto-opens the editor
        emit captureFinished(bundlePath);
    } else {
        core::logging::Logger::error("Failed to import external media from: " + cleanPath.toStdString());
    }
}

void CaptureController::duplicateProject(const QString& projectPath) {
    if (!m_libraryManager) return;
    
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    
    QString newBundlePath = QString("%1/.recordings/Copy_%2.ssa").arg(m_appDataDir).arg(ms);
    
    if (m_projectManager.duplicateProject(projectPath.toStdString(), newBundlePath.toStdString())) {
        core::logging::Logger::info("Successfully duplicated project to: " + newBundlePath.toStdString());
        
        library::MediaItem item;
        item.type = library::MediaType::Recording;
        item.path = newBundlePath;
        item.thumbnailPath = "";
        item.timestamp = ms;
        m_libraryManager->addMediaItem(item);
    } else {
        core::logging::Logger::error("Failed to duplicate project: " + projectPath.toStdString());
    }
}

void CaptureController::deleteProject(const QString& projectPath) {
    if (!m_libraryManager) return;
    
    // First remove from database so it disappears from UI instantly
    QList<library::MediaItem> items = m_libraryManager->getAllItems();
    for (const auto& item : items) {
        if (item.path == projectPath) {
            m_libraryManager->removeMediaItem(item.id);
            break;
        }
    }
    
    // Then physically delete the folder
    QDir dir(projectPath);
    if (dir.exists()) {
        dir.removeRecursively();
        core::logging::Logger::info("Deleted project bundle: " + projectPath.toStdString());
    }
}

} // namespace ssa::capture
