#pragma once

#include <QObject>
#include <QSqlDatabase>
#include <QString>
#include <QList>
#include <cstdint>
#include <memory>

namespace ssa::library {

enum class MediaType {
    Recording,
    Screenshot
};

struct MediaItem {
    int id = -1;
    MediaType type = MediaType::Recording;
    QString path;
    uint64_t timestamp = 0;
    int durationMs = 0; // 0 for screenshots
    QString resolution;
    QString thumbnailPath;
    QString tags;
};

class LibraryManager : public QObject {
    Q_OBJECT

public:
    explicit LibraryManager(QObject* parent = nullptr);
    ~LibraryManager() override;

    bool initialize(const QString& dbPath);
    
    bool addMediaItem(const MediaItem& item);
    bool removeMediaItem(int id);
    QList<MediaItem> getAllItems() const;
    
    // Scans directories and populates the DB with anything missing
    void scanExistingFiles(const QString& recordingsDir, const QString& screenshotsDir);

signals:
    void libraryUpdated();

private:
    QSqlDatabase m_db;
    QString m_dbPath;
    
    bool createTables();
};

} // namespace ssa::library
