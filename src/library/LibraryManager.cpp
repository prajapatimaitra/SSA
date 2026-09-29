#include "LibraryManager.h"
#include "core/logging/Logger.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QUuid>

namespace ssa::library {

LibraryManager::LibraryManager(QObject* parent) : QObject(parent) {
}

LibraryManager::~LibraryManager() {
    if (m_db.isOpen()) {
        m_db.close();
    }
}

bool LibraryManager::initialize(const QString& dbPath) {
    m_dbPath = dbPath;
    
    // Ensure directory exists
    QFileInfo dbFile(dbPath);
    QDir().mkpath(dbFile.absolutePath());

    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(dbPath);

    if (!m_db.open()) {
        core::logging::Logger::error("Failed to open library database: " + m_db.lastError().text().toStdString());
        return false;
    }

    core::logging::Logger::info("Library database opened successfully at " + dbPath.toStdString());
    return createTables();
}

bool LibraryManager::createTables() {
    QSqlQuery query(m_db);
    QString createTableStr = R"(
        CREATE TABLE IF NOT EXISTS media_items (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            type TEXT NOT NULL,
            path TEXT UNIQUE NOT NULL,
            timestamp INTEGER NOT NULL,
            duration_ms INTEGER DEFAULT 0,
            resolution TEXT,
            thumbnail_path TEXT,
            tags TEXT
        )
    )";

    if (!query.exec(createTableStr)) {
        core::logging::Logger::error("Failed to create media_items table: " + query.lastError().text().toStdString());
        return false;
    }

    return true;
}

bool LibraryManager::addMediaItem(const MediaItem& item) {
    if (!m_db.isOpen()) return false;

    QSqlQuery query(m_db);
    query.prepare(R"(
        INSERT OR REPLACE INTO media_items 
        (type, path, timestamp, duration_ms, resolution, thumbnail_path, tags) 
        VALUES (:type, :path, :timestamp, :duration_ms, :resolution, :thumbnail_path, :tags)
    )");

    query.bindValue(":type", item.type == MediaType::Recording ? "recording" : "screenshot");
    query.bindValue(":path", item.path);
    query.bindValue(":timestamp", static_cast<qint64>(item.timestamp));
    query.bindValue(":duration_ms", item.durationMs);
    query.bindValue(":resolution", item.resolution);
    query.bindValue(":thumbnail_path", item.thumbnailPath);
    query.bindValue(":tags", item.tags);

    if (!query.exec()) {
        core::logging::Logger::error("Failed to insert media item: " + query.lastError().text().toStdString());
        return false;
    }

    emit libraryUpdated();
    return true;
}

bool LibraryManager::removeMediaItem(int id) {
    if (!m_db.isOpen()) return false;

    QSqlQuery query(m_db);
    query.prepare("DELETE FROM media_items WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        core::logging::Logger::error("Failed to delete media item: " + query.lastError().text().toStdString());
        return false;
    }

    emit libraryUpdated();
    return true;
}

QList<MediaItem> LibraryManager::getAllItems() const {
    QList<MediaItem> items;
    if (!m_db.isOpen()) return items;

    QSqlQuery query("SELECT id, type, path, timestamp, duration_ms, resolution, thumbnail_path, tags FROM media_items ORDER BY timestamp DESC", m_db);

    QList<int> idsToRemove;

    while (query.next()) {
        MediaItem item;
        item.id = query.value(0).toInt();
        QString typeStr = query.value(1).toString();
        item.type = (typeStr == "screenshot") ? MediaType::Screenshot : MediaType::Recording;
        item.path = query.value(2).toString();
        item.timestamp = query.value(3).toULongLong();
        item.durationMs = query.value(4).toInt();
        item.resolution = query.value(5).toString();
        item.thumbnailPath = query.value(6).toString();
        item.tags = query.value(7).toString();
        
        // Auto-cleanup: If the user deleted the file/folder manually, remove it from DB
        if (!QFileInfo(item.path).exists()) {
            idsToRemove.append(item.id);
            continue;
        }
        
        items.append(item);
    }
    
    // Perform cleanup for deleted items
    for (int id : idsToRemove) {
        // cast away const temporarily to perform auto-cleanup, or do it on a separate connection
        // Wait, getAllItems() is const, so we can't call removeMediaItem(id) directly on *this if it's non-const
        // Let's just execute a quick delete query directly
        QSqlQuery deleteQuery(m_db);
        deleteQuery.prepare("DELETE FROM media_items WHERE id = :id");
        deleteQuery.bindValue(":id", id);
        deleteQuery.exec();
    }

    return items;
}

void LibraryManager::scanExistingFiles(const QString& recordingsDir, const QString& screenshotsDir) {
    core::logging::Logger::info("Scanning for existing media files...");

    QDir rDir(recordingsDir);
    if (rDir.exists()) {
        QStringList recordingFilters;
        recordingFilters << "*.ssa";
        QFileInfoList recordings = rDir.entryInfoList(recordingFilters, QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QFileInfo& fi : recordings) {
            MediaItem item;
            item.type = MediaType::Recording;
            item.path = fi.absoluteFilePath();
            item.timestamp = fi.birthTime().toMSecsSinceEpoch();
            
            // For existing files, if they aren't in DB they get added
            addMediaItem(item);
            
            // Note: Since screenshots might be inside .ssa/screenshots, let's scan them too
            QDir subScreenshotsDir(fi.absoluteFilePath() + "/screenshots");
            if (subScreenshotsDir.exists()) {
                QStringList sFilters;
                sFilters << "*.png";
                QFileInfoList ssList = subScreenshotsDir.entryInfoList(sFilters, QDir::Files);
                for (const QFileInfo& ssFi : ssList) {
                    MediaItem ssItem;
                    ssItem.type = MediaType::Screenshot;
                    ssItem.path = ssFi.absoluteFilePath();
                    ssItem.timestamp = ssFi.birthTime().toMSecsSinceEpoch();
                    ssItem.thumbnailPath = ssFi.absoluteFilePath();
                    addMediaItem(ssItem);
                }
            }
        }
    }

    QDir sDir(screenshotsDir);
    if (sDir.exists()) {
        QStringList screenshotFilters;
        screenshotFilters << "*.png";
        QFileInfoList screenshots = sDir.entryInfoList(screenshotFilters, QDir::Files);
        for (const QFileInfo& fi : screenshots) {
            MediaItem item;
            item.type = MediaType::Screenshot;
            item.path = fi.absoluteFilePath();
            item.timestamp = fi.birthTime().toMSecsSinceEpoch();
            item.thumbnailPath = fi.absoluteFilePath();
            addMediaItem(item);
        }
    }
    
    core::logging::Logger::info("Media scan complete.");
}

} // namespace ssa::library
