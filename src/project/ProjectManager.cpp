#include "project/ProjectManager.h"
#include "project/MetadataSerializer.h"
#include "core/logging/Logger.h"
#include <QDir>
#include <QFile>
#include <QUuid>
#include <chrono>
#include <iostream>

namespace ssa::project {

ProjectManager::ProjectManager() {
}

bool ProjectManager::createNewProject(const std::string& path, int videoWidth, int videoHeight) {
    QDir dir;
    if (!dir.mkpath(QString::fromStdString(path))) {
        core::logging::Logger::error("Failed to create project directory: " + path);
        return false;
    }

    m_currentProjectPath = path;

    ProjectMetadata meta;
    meta.uuid = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
    
    auto now = std::chrono::system_clock::now();
    meta.createdAt = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();
    
    meta.videoWidth = videoWidth;
    meta.videoHeight = videoHeight;

    return saveMetadata(meta);
}

bool ProjectManager::createProjectFromExternalMedia(const std::string& sourceMediaPath, const std::string& destBundlePath) {
    QDir dir;
    if (!dir.mkpath(QString::fromStdString(destBundlePath))) {
        core::logging::Logger::error("Failed to create project directory: " + destBundlePath);
        return false;
    }

    // Determine destination filename (always video.mp4 for now to match Editor expectations)
    QString destFile = QString::fromStdString(destBundlePath) + "/video.mp4";
    
    // Chunked copy to show progress in terminal
    QFile srcFile(QString::fromStdString(sourceMediaPath));
    QFile dstFile(destFile);
    
    if (!srcFile.open(QIODevice::ReadOnly)) {
        core::logging::Logger::error("Failed to open source media for reading.");
        return false;
    }
    if (!dstFile.open(QIODevice::WriteOnly)) {
        core::logging::Logger::error("Failed to open destination media for writing.");
        return false;
    }
    
    qint64 totalSize = srcFile.size();
    qint64 copiedSize = 0;
    qint64 chunkSize = 10 * 1024 * 1024; // 10 MB chunks
    
    std::cout << "Importing media file (" << totalSize / (1024*1024) << " MB)..." << std::endl;
    
    while (!srcFile.atEnd()) {
        QByteArray buffer = srcFile.read(chunkSize);
        if (buffer.isEmpty()) break;
        
        qint64 written = dstFile.write(buffer);
        if (written < 0) {
            core::logging::Logger::error("Failed to write to destination media.");
            return false;
        }
        
        copiedSize += written;
        int percent = (int)((copiedSize * 100) / totalSize);
        std::cout << "\rProgress: " << percent << "%" << std::flush;
    }
    
    std::cout << "\nImport complete!" << std::endl;
    
    srcFile.close();
    dstFile.close();

    m_currentProjectPath = destBundlePath;

    ProjectMetadata meta;
    meta.uuid = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
    auto now = std::chrono::system_clock::now();
    meta.createdAt = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();
    
    // Default to 1080p, QMediaPlayer/FFmpeg will dynamically scale it later
    meta.videoWidth = 1920; 
    meta.videoHeight = 1080;
    
    // Set recordingStartTimestamp so offline renderer knows where time 0 is
    meta.recordingStartTimestamp = meta.createdAt; 

    return saveMetadata(meta);
}

bool ProjectManager::duplicateProject(const std::string& sourceBundlePath, const std::string& newBundlePath) {
    QDir dir;
    if (!dir.exists(QString::fromStdString(sourceBundlePath))) {
        core::logging::Logger::error("Source project does not exist: " + sourceBundlePath);
        return false;
    }

    if (!dir.mkpath(QString::fromStdString(newBundlePath))) {
        core::logging::Logger::error("Failed to create destination directory: " + newBundlePath);
        return false;
    }

    // Temporarily switch project path to load metadata from source
    std::string originalPath = m_currentProjectPath;
    m_currentProjectPath = sourceBundlePath;
    auto metaOpt = loadMetadata();
    m_currentProjectPath = originalPath;

    if (!metaOpt) {
        core::logging::Logger::error("Failed to load metadata from source project.");
        return false;
    }

    ProjectMetadata meta = metaOpt.value();
    
    // Generate new UUID and timestamp
    meta.uuid = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
    auto now = std::chrono::system_clock::now();
    meta.createdAt = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();

    // Set new project path and save modified metadata
    m_currentProjectPath = newBundlePath;
    if (!saveMetadata(meta)) {
        core::logging::Logger::error("Failed to save duplicated metadata.");
        return false;
    }

    // Hard-link media files
    QString srcVideo = QString::fromStdString(sourceBundlePath) + "/video.mp4";
    QString dstVideo = QString::fromStdString(newBundlePath) + "/video.mp4";
    if (QFile::exists(srcVideo)) {
        if (!QFile::link(srcVideo, dstVideo)) {
            core::logging::Logger::error("Failed to hard-link video.mp4. Falling back to copy.");
            QFile::copy(srcVideo, dstVideo);
        }
    }

    QString srcWebcam = QString::fromStdString(sourceBundlePath) + "/webcam.mp4";
    QString dstWebcam = QString::fromStdString(newBundlePath) + "/webcam.mp4";
    if (QFile::exists(srcWebcam)) {
        if (!QFile::link(srcWebcam, dstWebcam)) {
            QFile::copy(srcWebcam, dstWebcam);
        }
    }

    return true;
}

bool ProjectManager::saveMetadata(const ProjectMetadata& metadata) {
    if (m_currentProjectPath.empty()) return false;

    QString jsonStr = MetadataSerializer::serialize(metadata);
    QString metaPath = QString::fromStdString(m_currentProjectPath) + "/metadata.json";
    
    QFile file(metaPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        core::logging::Logger::error("Failed to open metadata.json for writing in " + m_currentProjectPath);
        return false;
    }

    file.write(jsonStr.toUtf8());
    file.close();
    
    core::logging::Logger::info("Successfully saved project metadata with " + 
                                std::to_string(metadata.mouseEvents.size()) + " mouse events.");
    return true;
}

std::optional<ProjectMetadata> ProjectManager::loadMetadata() {
    if (m_currentProjectPath.empty()) return std::nullopt;

    QString metaPath = QString::fromStdString(m_currentProjectPath) + "/metadata.json";
    QFile file(metaPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        core::logging::Logger::error("Failed to open metadata.json for reading in " + m_currentProjectPath);
        return std::nullopt;
    }

    QString jsonStr = file.readAll();
    file.close();

    return MetadataSerializer::deserialize(jsonStr);
}

} // namespace ssa::project
