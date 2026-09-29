#include "ExportQueueManager.h"
#include "core/logging/Logger.h"
#include <QFileInfo>

namespace ssa::export_engine {

ExportQueueManager::ExportQueueManager(QObject* parent)
    : QObject(parent) {
}

ExportQueueManager::~ExportQueueManager() {
    cancelCurrentExport();
}

void ExportQueueManager::enqueueExport(const QString& projectPath, const QString& presetName, const QString& resolution) {
    ExportJob job;
    job.projectPath = projectPath;
    if (job.projectPath.startsWith("file://")) {
        job.projectPath = job.projectPath.mid(7);
    }
    job.presetName = presetName;
    job.resolution = resolution;

    m_queue.enqueue(job);
    emit queueSizeChanged(m_queue.size());

    core::logging::Logger::info("Queued export job for: " + job.projectPath.toStdString());

    if (!m_isExporting) {
        processNextJob();
    }
}

void ExportQueueManager::cancelCurrentExport() {
    if (m_currentEngine) {
        m_currentEngine->cancelExport();
    }
    clearQueue();
}

void ExportQueueManager::clearQueue() {
    m_queue.clear();
    emit queueSizeChanged(0);
}

void ExportQueueManager::processNextJob() {
    if (m_queue.isEmpty()) {
        m_isExporting = false;
        m_currentProjectName = "";
        m_currentProgress = 0.0;
        emit isExportingChanged(false);
        emit currentProjectNameChanged("");
        emit currentProgressChanged(0.0);
        emit queueCompleted();
        return;
    }

    m_isExporting = true;
    emit isExportingChanged(true);

    ExportJob job = m_queue.dequeue();
    emit queueSizeChanged(m_queue.size());

    m_currentProjectName = QFileInfo(job.projectPath).fileName();
    emit currentProjectNameChanged(m_currentProjectName);

    m_currentProgress = 0.0;
    emit currentProgressChanged(0.0);

    core::logging::Logger::info("Starting queued export for: " + m_currentProjectName.toStdString());

    m_currentEngine = std::make_unique<ExportEngine>();
    connect(m_currentEngine.get(), &ExportEngine::progressChanged, this, &ExportQueueManager::onEngineProgress);
    connect(m_currentEngine.get(), &ExportEngine::exportFinished, this, &ExportQueueManager::onEngineFinished);

    QString outputPath = job.projectPath + "/final_export.mp4";
    m_currentEngine->startExport(job.projectPath, job.presetName, job.resolution, outputPath);
}

void ExportQueueManager::onEngineProgress(double progress) {
    m_currentProgress = progress;
    emit currentProgressChanged(progress);
}

void ExportQueueManager::onEngineFinished(bool success, const QString& path) {
    if (success) {
        core::logging::Logger::info("Export job completed successfully: " + path.toStdString());
    } else {
        core::logging::Logger::error("Export job failed: " + path.toStdString());
    }

    emit jobFinished(success, path);

    // Ensure the engine thread is cleanly detached/joined by destroying it after a tiny delay
    m_currentEngine->deleteLater();
    m_currentEngine.release(); // Hand over ownership to Qt's event loop

    // Process next job
    processNextJob();
}

} // namespace ssa::export_engine
