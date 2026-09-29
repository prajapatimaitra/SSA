#include "AutoSaveManager.h"
#include "EditorController.h"
#include "project/MetadataSerializer.h"
#include "core/logging/Logger.h"
#include <QFile>
#include <QDir>

namespace ssa::editor {

AutoSaveManager::AutoSaveManager(EditorController* controller, QObject* parent)
    : QObject(parent), m_controller(controller) 
{
    // Fire every 15 seconds
    m_timer.setInterval(15000);
    connect(&m_timer, &QTimer::timeout, this, &AutoSaveManager::performAutoSave);
}

AutoSaveManager::~AutoSaveManager() {
    stopAndCommit();
}

void AutoSaveManager::start(const QString& projectPath) {
    m_projectPath = projectPath;
    m_timer.start();
}

void AutoSaveManager::stopAndCommit() {
    m_timer.stop();
    if (m_projectPath.isEmpty()) return;

    QString autosavePath = m_projectPath + "/metadata.json.autosave";
    if (QFile::exists(autosavePath)) {
        // When stopping cleanly (e.g. closing the editor), we assume EditorController::saveProject() 
        // handles writing to metadata.json. We just clean up the autosave file to indicate no crash.
        QFile::remove(autosavePath);
        core::logging::Logger::info("Clean exit, removed autosave file.");
    }
    m_projectPath.clear();
}

void AutoSaveManager::performAutoSave() {
    if (m_projectPath.isEmpty() || !m_controller) return;

    auto metadataOpt = m_controller->currentMetadata();
    if (!metadataOpt) return;

    QString autosavePath = m_projectPath + "/metadata.json.autosave";
    QString jsonStr = project::MetadataSerializer::serialize(*metadataOpt);

    // Save to a temporary file first, then rename, to avoid corruption if crash happens exactly during write
    QString tempPath = autosavePath + ".tmp";
    QFile file(tempPath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        file.write(jsonStr.toUtf8());
        file.close();
        
        // Atomic rename
        if (QFile::exists(autosavePath)) {
            QFile::remove(autosavePath);
        }
        QFile::rename(tempPath, autosavePath);
        
        core::logging::Logger::info("Autosave complete: " + autosavePath.toStdString());
    }
}

} // namespace ssa::editor
