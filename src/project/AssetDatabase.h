#pragma once

#include <QObject>
#include <QString>
#include <QList>
#include <QSqlDatabase>
#include <cstdint>

namespace ssa::project {

struct ProjectBin {
    int binId = -1;
    int parentBinId = -1; // -1 for root
    QString name;
    QString colorTag = "#4f46e5";
    uint64_t createdAt = 0;
};

struct ProjectAsset {
    int assetId = -1;
    int binId = 1;
    QString name;
    QString filePath;
    QString mediaType; // "video", "audio", "image"
    int64_t durationMs = 0;
    int width = 0;
    int height = 0;
    double fps = 0.0;
    int sampleRate = 0;
    int channels = 0;
    QString thumbnailPath;
    int64_t fileSizeBytes = 0;
    uint64_t createdAt = 0;
};

class AssetDatabase : public QObject {
    Q_OBJECT

public:
    explicit AssetDatabase(QObject* parent = nullptr);
    ~AssetDatabase() override;

    bool initialize(const QString& dbPath);
    bool isOpen() const;

    // Bin Management
    int createBin(const QString& name, int parentBinId = 1, const QString& colorTag = "#4f46e5");
    bool renameBin(int binId, const QString& newName);
    bool updateBinColor(int binId, const QString& newColorTag);
    bool deleteBin(int binId);
    QList<ProjectBin> getAllBins() const;
    ProjectBin getBinById(int binId) const;

    // Asset Management
    int addAsset(const ProjectAsset& asset);
    bool removeAsset(int assetId);
    bool moveAssetToBin(int assetId, int targetBinId);
    QList<ProjectAsset> getAssetsInBin(int binId) const;
    QList<ProjectAsset> getAllAssets() const;
    ProjectAsset getAssetById(int assetId) const;
    QList<ProjectAsset> searchAssets(const QString& query) const;

    // Helper: Import external file into appDataDir/.recordings folder and parse metadata
    int importMediaFile(const QString& sourceFilePath, int binId, const QString& appDataDir);

signals:
    void databaseUpdated();
    void binAdded(int binId);
    void binRemoved(int binId);
    void assetAdded(int assetId);
    void assetRemoved(int assetId);

private:
    QSqlDatabase m_db;
    QString m_dbPath;

    bool createTables();
};

} // namespace ssa::project
