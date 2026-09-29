#pragma once

#include <QObject>
#include <QString>
#include <QMediaPlayer>
#include <QVideoSink>
#include <QVideoFrame>
#include <memory>
#include <thread>
#include <atomic>
#include "export/interfaces/IMediaEncoder.h"
#include "export/interfaces/IMediaDecoder.h"
#include "project/ProjectMetadata.h"
#include "editor/PlaybackEngine.h"

namespace ssa::editor { class EditorController; }

namespace ssa::export_engine {

class ExportEngine : public QObject {
    Q_OBJECT
    Q_PROPERTY(double progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(bool isExporting READ isExporting NOTIFY isExportingChanged)

public:
    explicit ExportEngine(QObject* parent = nullptr);
    ~ExportEngine();

    Q_INVOKABLE void startExport(const QString& projectPath, const QString& presetName, const QString& resolution, const QString& outputPath);
    Q_INVOKABLE void cancelExport();

    double progress() const { return m_progress; }
    bool isExporting() const { return m_isExporting; }

signals:
    void progressChanged(double progress);
    void isExportingChanged(bool isExporting);
    void exportFinished(bool success, const QString& path);

private:
    void runExportLoop();
    void finishExport(bool success);

    std::unique_ptr<editor::PlaybackEngine> m_playbackEngine;
    project::ProjectMetadata m_metadata;
    QString m_projectPath;

    std::unique_ptr<IMediaEncoder> m_encoder;
    std::unique_ptr<IMediaDecoder> m_decoder;
    std::unique_ptr<IMediaDecoder> m_webcamDecoder;
    
    std::atomic<bool> m_isExporting{false};
    std::atomic<bool> m_cancelRequested{false};
    std::thread m_exportThread;
    
    double m_progress = 0.0;
    QString m_outputPath;
    
    qint64 m_totalDurationMs = 0;
    int m_fps = 60;
};

} // namespace ssa::export_engine
