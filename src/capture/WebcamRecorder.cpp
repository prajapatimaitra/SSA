#include "WebcamRecorder.h"
#include "core/logging/Logger.h"
#include <QUrl>
#include <QMediaDevices>
#include <QMediaFormat>
#include <QCameraDevice>

namespace ssa::capture {

WebcamRecorder::WebcamRecorder(QObject* parent) : QObject(parent) {
    m_captureSession = std::make_unique<QMediaCaptureSession>();
    m_recorder = std::make_unique<QMediaRecorder>();
    m_captureSession->setRecorder(m_recorder.get());
}

WebcamRecorder::~WebcamRecorder() {
    stopRecording();
}

void WebcamRecorder::startRecording(const QByteArray& cameraId, const QString& outputPath) {
    if (cameraId.isEmpty()) {
        return;
    }
    
    const auto cameras = QMediaDevices::videoInputs();
    QCameraDevice selectedDevice;
    for (const auto& camera : cameras) {
        if (camera.id() == cameraId) {
            selectedDevice = camera;
            break;
        }
    }
    
    if (selectedDevice.isNull()) {
        core::logging::Logger::error("Webcam device not found: " + cameraId.toStdString());
        return;
    }

    m_camera = std::make_unique<QCamera>(selectedDevice);
    m_captureSession->setCamera(m_camera.get());
    
    m_recorder->setOutputLocation(QUrl::fromLocalFile(outputPath));
    
    // Choose MP4 container
    QMediaFormat format(QMediaFormat::MPEG4);
    format.setVideoCodec(QMediaFormat::VideoCodec::H264);
    m_recorder->setMediaFormat(format);
    
    m_camera->start();
    m_recorder->record();
    
    core::logging::Logger::info("Started webcam recording to " + outputPath.toStdString());
}

void WebcamRecorder::stopRecording() {
    if (m_recorder && m_recorder->recorderState() == QMediaRecorder::RecordingState) {
        m_recorder->stop();
    }
    if (m_camera && m_camera->isActive()) {
        m_camera->stop();
    }
    m_camera.reset();
}

} // namespace ssa::capture
