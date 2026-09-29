#pragma once
#include <QObject>
#include <QCamera>
#include <QMediaCaptureSession>
#include <QMediaRecorder>
#include <memory>
#include <QString>

namespace ssa::capture {

class WebcamRecorder : public QObject {
    Q_OBJECT
public:
    explicit WebcamRecorder(QObject* parent = nullptr);
    ~WebcamRecorder();

    void startRecording(const QByteArray& cameraId, const QString& outputPath);
    void stopRecording();

private:
    std::unique_ptr<QCamera> m_camera;
    std::unique_ptr<QMediaCaptureSession> m_captureSession;
    std::unique_ptr<QMediaRecorder> m_recorder;
};

} // namespace ssa::capture
