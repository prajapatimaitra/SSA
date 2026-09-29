#include "project/AssetDatabase.h"
#include "core/logging/Logger.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QFileInfo>
#include <QDir>
#include <QFile>
#include <QImageReader>
#include <QImage>
#include <QUuid>
#include <chrono>

namespace ssa::project {

AssetDatabase::AssetDatabase(QObject* parent)
    : QObject(parent)
{
}

AssetDatabase::~AssetDatabase() {
    if (m_db.isOpen()) {
        m_db.close();
    }
}

bool AssetDatabase::initialize(const QString& dbPath) {
    m_dbPath = dbPath;
    
    // SQLite connection name to prevent collisions
    QString connName = "ssa_asset_db_" + QString::number(reinterpret_cast<quintptr>(this));
    m_db = QSqlDatabase::addDatabase("QSQLITE", connName);
    m_db.setDatabaseName(dbPath);

    if (!m_db.open()) {
        core::logging::Logger::error("Failed to open AssetDatabase: " + m_db.lastError().text().toStdString());
        return false;
    }

    core::logging::Logger::info("AssetDatabase initialized at: " + dbPath.toStdString());
    return createTables();
}

bool AssetDatabase::isOpen() const {
    return m_db.isOpen();
}

bool AssetDatabase::createTables() {
    if (!m_db.isOpen()) return false;

    QSqlQuery query(m_db);

    // Create Bins Table
    bool ok = query.exec(
        "CREATE TABLE IF NOT EXISTS project_bins ("
        "bin_id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "parent_bin_id INTEGER DEFAULT -1, "
        "name TEXT NOT NULL, "
        "color_tag TEXT DEFAULT '#4f46e5', "
        "created_at INTEGER NOT NULL"
        ");"
    );

    if (!ok) {
        core::logging::Logger::error("Failed to create project_bins table: " + query.lastError().text().toStdString());
        return false;
    }

    // Ensure Master / Root Bin exists (ID 1)
    query.exec("SELECT COUNT(*) FROM project_bins WHERE bin_id = 1;");
    if (query.next() && query.value(0).toInt() == 0) {
        uint64_t now = QDateTime::currentMSecsSinceEpoch();
        QSqlQuery insertMaster(m_db);
        insertMaster.prepare("INSERT INTO project_bins (bin_id, parent_bin_id, name, color_tag, created_at) "
                             "VALUES (1, -1, 'Master', '#6366f1', :created);");
        insertMaster.bindValue(":created", static_cast<qint64>(now));
        insertMaster.exec();
    }

    // Create Assets Table
    ok = query.exec(
        "CREATE TABLE IF NOT EXISTS project_assets ("
        "asset_id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "bin_id INTEGER DEFAULT 1, "
        "name TEXT NOT NULL, "
        "file_path TEXT NOT NULL, "
        "media_type TEXT NOT NULL, "
        "duration_ms INTEGER DEFAULT 0, "
        "width INTEGER DEFAULT 0, "
        "height INTEGER DEFAULT 0, "
        "fps REAL DEFAULT 0.0, "
        "sample_rate INTEGER DEFAULT 0, "
        "channels INTEGER DEFAULT 0, "
        "thumbnail_path TEXT, "
        "file_size_bytes INTEGER DEFAULT 0, "
        "created_at INTEGER NOT NULL, "
        "FOREIGN KEY(bin_id) REFERENCES project_bins(bin_id) ON DELETE CASCADE"
        ");"
    );

    if (!ok) {
        core::logging::Logger::error("Failed to create project_assets table: " + query.lastError().text().toStdString());
        return false;
    }

    return true;
}

int AssetDatabase::createBin(const QString& name, int parentBinId, const QString& colorTag) {
    if (!m_db.isOpen()) return -1;

    QSqlQuery query(m_db);
    query.prepare("INSERT INTO project_bins (parent_bin_id, name, color_tag, created_at) "
                  "VALUES (:parent, :name, :color, :created);");
    query.bindValue(":parent", parentBinId);
    query.bindValue(":name", name);
    query.bindValue(":color", colorTag);
    query.bindValue(":created", static_cast<qint64>(QDateTime::currentMSecsSinceEpoch()));

    if (!query.exec()) {
        core::logging::Logger::error("Failed to create bin: " + query.lastError().text().toStdString());
        return -1;
    }

    int newId = query.lastInsertId().toInt();
    emit binAdded(newId);
    emit databaseUpdated();
    return newId;
}

bool AssetDatabase::renameBin(int binId, const QString& newName) {
    if (!m_db.isOpen()) return false;

    QSqlQuery query(m_db);
    query.prepare("UPDATE project_bins SET name = :name WHERE bin_id = :id;");
    query.bindValue(":name", newName);
    query.bindValue(":id", binId);

    if (query.exec()) {
        emit databaseUpdated();
        return true;
    }
    return false;
}

bool AssetDatabase::updateBinColor(int binId, const QString& newColorTag) {
    if (!m_db.isOpen()) return false;

    QSqlQuery query(m_db);
    query.prepare("UPDATE project_bins SET color_tag = :color WHERE bin_id = :id;");
    query.bindValue(":color", newColorTag);
    query.bindValue(":id", binId);

    if (query.exec()) {
        emit databaseUpdated();
        return true;
    }
    return false;
}

bool AssetDatabase::deleteBin(int binId) {
    if (!m_db.isOpen() || binId <= 1) return false; // Don't delete Master bin

    // Reassign assets in deleted bin to Master bin (bin_id = 1)
    QSqlQuery reassign(m_db);
    reassign.prepare("UPDATE project_assets SET bin_id = 1 WHERE bin_id = :id;");
    reassign.bindValue(":id", binId);
    reassign.exec();

    // Reassign sub-bins to parent of deleted bin
    ProjectBin currentBin = getBinById(binId);
    QSqlQuery reassignSub(m_db);
    reassignSub.prepare("UPDATE project_bins SET parent_bin_id = :parent WHERE parent_bin_id = :id;");
    reassignSub.bindValue(":parent", currentBin.parentBinId);
    reassignSub.bindValue(":id", binId);
    reassignSub.exec();

    QSqlQuery query(m_db);
    query.prepare("DELETE FROM project_bins WHERE bin_id = :id;");
    query.bindValue(":id", binId);

    if (query.exec()) {
        emit binRemoved(binId);
        emit databaseUpdated();
        return true;
    }
    return false;
}

QList<ProjectBin> AssetDatabase::getAllBins() const {
    QList<ProjectBin> bins;
    if (!m_db.isOpen()) return bins;

    QSqlQuery query("SELECT bin_id, parent_bin_id, name, color_tag, created_at FROM project_bins ORDER BY parent_bin_id ASC, name ASC;", m_db);
    while (query.next()) {
        ProjectBin bin;
        bin.binId = query.value(0).toInt();
        bin.parentBinId = query.value(1).toInt();
        bin.name = query.value(2).toString();
        bin.colorTag = query.value(3).toString();
        bin.createdAt = query.value(4).toULongLong();
        bins.append(bin);
    }
    return bins;
}

ProjectBin AssetDatabase::getBinById(int binId) const {
    ProjectBin bin;
    if (!m_db.isOpen()) return bin;

    QSqlQuery query(m_db);
    query.prepare("SELECT bin_id, parent_bin_id, name, color_tag, created_at FROM project_bins WHERE bin_id = :id;");
    query.bindValue(":id", binId);
    if (query.exec() && query.next()) {
        bin.binId = query.value(0).toInt();
        bin.parentBinId = query.value(1).toInt();
        bin.name = query.value(2).toString();
        bin.colorTag = query.value(3).toString();
        bin.createdAt = query.value(4).toULongLong();
    }
    return bin;
}

int AssetDatabase::addAsset(const ProjectAsset& asset) {
    if (!m_db.isOpen()) return -1;

    QSqlQuery query(m_db);
    query.prepare(
        "INSERT INTO project_assets ("
        "bin_id, name, file_path, media_type, duration_ms, width, height, fps, sample_rate, channels, thumbnail_path, file_size_bytes, created_at"
        ") VALUES ("
        ":bin, :name, :path, :type, :duration, :width, :height, :fps, :sample_rate, :channels, :thumb, :size, :created"
        ");"
    );

    query.bindValue(":bin", asset.binId);
    query.bindValue(":name", asset.name);
    query.bindValue(":path", asset.filePath);
    query.bindValue(":type", asset.mediaType);
    query.bindValue(":duration", static_cast<qint64>(asset.durationMs));
    query.bindValue(":width", asset.width);
    query.bindValue(":height", asset.height);
    query.bindValue(":fps", asset.fps);
    query.bindValue(":sample_rate", asset.sampleRate);
    query.bindValue(":channels", asset.channels);
    query.bindValue(":thumb", asset.thumbnailPath);
    query.bindValue(":size", static_cast<qint64>(asset.fileSizeBytes));
    query.bindValue(":created", static_cast<qint64>(asset.createdAt > 0 ? asset.createdAt : QDateTime::currentMSecsSinceEpoch()));

    if (!query.exec()) {
        core::logging::Logger::error("Failed to add asset: " + query.lastError().text().toStdString());
        return -1;
    }

    int newId = query.lastInsertId().toInt();
    emit assetAdded(newId);
    emit databaseUpdated();
    return newId;
}

bool AssetDatabase::removeAsset(int assetId) {
    if (!m_db.isOpen()) return false;

    QSqlQuery query(m_db);
    query.prepare("DELETE FROM project_assets WHERE asset_id = :id;");
    query.bindValue(":id", assetId);

    if (query.exec()) {
        emit assetRemoved(assetId);
        emit databaseUpdated();
        return true;
    }
    return false;
}

bool AssetDatabase::moveAssetToBin(int assetId, int targetBinId) {
    if (!m_db.isOpen()) return false;

    QSqlQuery query(m_db);
    query.prepare("UPDATE project_assets SET bin_id = :bin WHERE asset_id = :id;");
    query.bindValue(":bin", targetBinId);
    query.bindValue(":id", assetId);

    if (query.exec()) {
        emit databaseUpdated();
        return true;
    }
    return false;
}

QList<ProjectAsset> AssetDatabase::getAssetsInBin(int binId) const {
    QList<ProjectAsset> assets;
    if (!m_db.isOpen()) return assets;

    QSqlQuery query(m_db);
    query.prepare("SELECT asset_id, bin_id, name, file_path, media_type, duration_ms, width, height, fps, sample_rate, channels, thumbnail_path, file_size_bytes, created_at FROM project_assets WHERE bin_id = :bin ORDER BY created_at DESC;");
    query.bindValue(":bin", binId);

    if (query.exec()) {
        while (query.next()) {
            ProjectAsset asset;
            asset.assetId = query.value(0).toInt();
            asset.binId = query.value(1).toInt();
            asset.name = query.value(2).toString();
            asset.filePath = query.value(3).toString();
            asset.mediaType = query.value(4).toString();
            asset.durationMs = query.value(5).toLongLong();
            asset.width = query.value(6).toInt();
            asset.height = query.value(7).toInt();
            asset.fps = query.value(8).toDouble();
            asset.sampleRate = query.value(9).toInt();
            asset.channels = query.value(10).toInt();
            asset.thumbnailPath = query.value(11).toString();
            asset.fileSizeBytes = query.value(12).toLongLong();
            asset.createdAt = query.value(13).toULongLong();
            assets.append(asset);
        }
    }
    return assets;
}

QList<ProjectAsset> AssetDatabase::getAllAssets() const {
    QList<ProjectAsset> assets;
    if (!m_db.isOpen()) return assets;

    QSqlQuery query("SELECT asset_id, bin_id, name, file_path, media_type, duration_ms, width, height, fps, sample_rate, channels, thumbnail_path, file_size_bytes, created_at FROM project_assets ORDER BY created_at DESC;", m_db);
    while (query.next()) {
        ProjectAsset asset;
        asset.assetId = query.value(0).toInt();
        asset.binId = query.value(1).toInt();
        asset.name = query.value(2).toString();
        asset.filePath = query.value(3).toString();
        asset.mediaType = query.value(4).toString();
        asset.durationMs = query.value(5).toLongLong();
        asset.width = query.value(6).toInt();
        asset.height = query.value(7).toInt();
        asset.fps = query.value(8).toDouble();
        asset.sampleRate = query.value(9).toInt();
        asset.channels = query.value(10).toInt();
        asset.thumbnailPath = query.value(11).toString();
        asset.fileSizeBytes = query.value(12).toLongLong();
        asset.createdAt = query.value(13).toULongLong();
        assets.append(asset);
    }
    return assets;
}

ProjectAsset AssetDatabase::getAssetById(int assetId) const {
    ProjectAsset asset;
    if (!m_db.isOpen()) return asset;

    QSqlQuery query(m_db);
    query.prepare("SELECT asset_id, bin_id, name, file_path, media_type, duration_ms, width, height, fps, sample_rate, channels, thumbnail_path, file_size_bytes, created_at FROM project_assets WHERE asset_id = :id;");
    query.bindValue(":id", assetId);

    if (query.exec() && query.next()) {
        asset.assetId = query.value(0).toInt();
        asset.binId = query.value(1).toInt();
        asset.name = query.value(2).toString();
        asset.filePath = query.value(3).toString();
        asset.mediaType = query.value(4).toString();
        asset.durationMs = query.value(5).toLongLong();
        asset.width = query.value(6).toInt();
        asset.height = query.value(7).toInt();
        asset.fps = query.value(8).toDouble();
        asset.sampleRate = query.value(9).toInt();
        asset.channels = query.value(10).toInt();
        asset.thumbnailPath = query.value(11).toString();
        asset.fileSizeBytes = query.value(12).toLongLong();
        asset.createdAt = query.value(13).toULongLong();
    }
    return asset;
}

QList<ProjectAsset> AssetDatabase::searchAssets(const QString& queryStr) const {
    QList<ProjectAsset> assets;
    if (!m_db.isOpen() || queryStr.isEmpty()) return assets;

    QSqlQuery query(m_db);
    query.prepare("SELECT asset_id, bin_id, name, file_path, media_type, duration_ms, width, height, fps, sample_rate, channels, thumbnail_path, file_size_bytes, created_at FROM project_assets WHERE name LIKE :q ORDER BY name ASC;");
    query.bindValue(":q", "%" + queryStr + "%");

    if (query.exec()) {
        while (query.next()) {
            ProjectAsset asset;
            asset.assetId = query.value(0).toInt();
            asset.binId = query.value(1).toInt();
            asset.name = query.value(2).toString();
            asset.filePath = query.value(3).toString();
            asset.mediaType = query.value(4).toString();
            asset.durationMs = query.value(5).toLongLong();
            asset.width = query.value(6).toInt();
            asset.height = query.value(7).toInt();
            asset.fps = query.value(8).toDouble();
            asset.sampleRate = query.value(9).toInt();
            asset.channels = query.value(10).toInt();
            asset.thumbnailPath = query.value(11).toString();
            asset.fileSizeBytes = query.value(12).toLongLong();
            asset.createdAt = query.value(13).toULongLong();
            assets.append(asset);
        }
    }
    return assets;
}

int AssetDatabase::importMediaFile(const QString& sourceFilePath, int binId, const QString& appDataDir) {
    QFileInfo sourceInfo(sourceFilePath);
    if (!sourceInfo.exists()) {
        core::logging::Logger::error("Import failed: source file does not exist: " + sourceFilePath.toStdString());
        return -1;
    }

    // Target directory: appDataDir + "/.recordings" (as confirmed by user)
    QString recordingsDir = appDataDir + "/.recordings";
    QDir().mkpath(recordingsDir);

    // Generate unique dest filename
    QString ext = sourceInfo.suffix().toLower();
    QString uniqueName = QUuid::createUuid().toString(QUuid::WithoutBraces) + "." + ext;
    QString destFilePath = recordingsDir + "/" + uniqueName;

    // Copy source media into .recordings folder if not already inside it
    if (QDir::cleanPath(sourceInfo.absoluteFilePath()) != QDir::cleanPath(destFilePath)) {
        if (!QFile::copy(sourceFilePath, destFilePath)) {
            core::logging::Logger::error("Failed to copy media file to recordings dir: " + destFilePath.toStdString());
            // fallback to original path if copy fails
            destFilePath = sourceInfo.absoluteFilePath();
        }
    }

    ProjectAsset asset;
    asset.binId = binId > 0 ? binId : 1;
    asset.name = sourceInfo.fileName();
    asset.filePath = destFilePath;
    asset.fileSizeBytes = sourceInfo.size();
    asset.createdAt = QDateTime::currentMSecsSinceEpoch();

    // Determine media type and extract dimensions/duration where possible
    if (ext == "mp4" || ext == "mov" || ext == "mkv" || ext == "avi" || ext == "webm") {
        asset.mediaType = "video";
        asset.width = 1920;
        asset.height = 1080;
        asset.fps = 30.0;
        asset.durationMs = 10000; // default initial duration until playback probe
    } else if (ext == "mp3" || ext == "wav" || ext == "m4a" || ext == "aac" || ext == "flac") {
        asset.mediaType = "audio";
        asset.sampleRate = 44100;
        asset.channels = 2;
        asset.durationMs = 5000;
    } else if (ext == "png" || ext == "jpg" || ext == "jpeg" || ext == "webp" || ext == "svg") {
        asset.mediaType = "image";
        QImageReader reader(destFilePath);
        QSize sz = reader.size();
        if (sz.isValid()) {
            asset.width = sz.width();
            asset.height = sz.height();
        } else {
            asset.width = 1920;
            asset.height = 1080;
        }
        asset.durationMs = 5000; // Still images defaulted to 5 sec clip length
        asset.thumbnailPath = destFilePath;
    } else {
        asset.mediaType = "video";
    }

    return addAsset(asset);
}

} // namespace ssa::project
