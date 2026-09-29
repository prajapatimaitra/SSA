#pragma once

#include <QObject>
#include <QString>
#include <memory>
#include "capture/interfaces/IPlatformCapture.h"
#include "capture/interfaces/IScreenCapturer.h"

#include "input/interfaces/IInputTracker.h"
#include "project/ProjectManager.h"
#include "WebcamRecorder.h"
#include "ai/AIEngine.h"
#include <vector>
#include <mutex>

#include <QMediaCaptureSession>
#include <QAudioInput>
#include <QMediaRecorder>

namespace ssa::library {
class LibraryManager;
}

namespace ssa::capture {

class CaptureController : public QObject {
    Q_OBJECT

public:
    explicit CaptureController(std::unique_ptr<IPlatformCapture> captureEngine, 
                               std::unique_ptr<IScreenCapturer> screenCapturer,
                               std::unique_ptr<input::IInputTracker> inputTracker,
                               QObject* parent = nullptr);

    Q_INVOKABLE void startCapture(const QString& presetName, const QString& displayId, const QByteArray& cameraId = QByteArray(), int fps = 60, const QString& resolution = "1080p", int audioSource = 2, int cropX = 0, int cropY = 0, int cropW = 0, int cropH = 0);
    Q_INVOKABLE void stopCapture();
    Q_INVOKABLE void captureScreenshot();
    Q_INVOKABLE void importExternalMedia(const QString& mediaPath);
    Q_INVOKABLE void duplicateProject(const QString& projectPath);
    Q_INVOKABLE void deleteProject(const QString& projectPath);
    
    void setLibraryManager(class ssa::library::LibraryManager* lm) { m_libraryManager = lm; }
    void setAppDataDir(const QString& dir) { m_appDataDir = dir; }
    QString appDataDir() const { return m_appDataDir; }

signals:
    void captureFinished(const QString& projectPath);
    void screenshotCaptured(const QString& imagePath);
    void recordingStarted();
    void recordingStopped();
    void subtitlesReady(const QString& vttPath);

private:
    std::unique_ptr<IPlatformCapture> m_captureEngine;
    std::unique_ptr<IScreenCapturer> m_screenCapturer;
    std::unique_ptr<input::IInputTracker> m_inputTracker;
    
    project::ProjectManager m_projectManager;
    std::vector<input::MouseEvent> m_recordedEvents;
    std::mutex m_eventsMutex;
    
    input::MouseEventType m_lastEventType;
    int m_lastVideoWidth = 0;
    int m_lastVideoHeight = 0;
    uint64_t m_firstFrameTimestamp = 0;
    std::string m_projectPath;
    
    QByteArray m_selectedWebcamId;
    std::unique_ptr<WebcamRecorder> m_webcamRecorder;
    
    std::unique_ptr<class QMediaCaptureSession> m_micCaptureSession;
    std::unique_ptr<class QAudioInput> m_micAudioInput;
    std::unique_ptr<class QMediaRecorder> m_micRecorder;
    
    std::unique_ptr<ai::AIEngine> m_aiEngine;
    
    bool m_isStopping = false;
    bool m_isRecording = false;
    
    QString m_appDataDir;
    class ssa::library::LibraryManager* m_libraryManager = nullptr;
};

} // namespace ssa::capture
