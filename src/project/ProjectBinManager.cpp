#include "project/ProjectBinManager.h"
#include "core/logging/Logger.h"
#include <QUrl>

namespace ssa::project {

ProjectBinManager::ProjectBinManager(QObject* parent)
    : QObject(parent)
{
}

bool ProjectBinManager::initialize(const QString& dbPath, const QString& appDataDir) {
    m_appDataDir = appDataDir;

    if (!m_db.initialize(dbPath)) {
        core::logging::Logger::error("ProjectBinManager: Failed to initialize AssetDatabase.");
        return false;
    }

    m_binModel = std::make_unique<ProjectBinModel>(&m_db, this);
    m_assetModel = std::make_unique<AssetListModel>(&m_db, this);

    connect(&m_db, &AssetDatabase::databaseUpdated, this, &ProjectBinManager::statsChanged);

    emit modelsChanged();
    emit statsChanged();

    core::logging::Logger::info("ProjectBinManager initialized successfully.");
    return true;
}

void ProjectBinManager::setSelectedBinId(int binId) {
    if (m_selectedBinId != binId) {
        m_selectedBinId = binId;
        if (m_assetModel) {
            m_assetModel->setCurrentBinId(binId);
        }
        emit selectedBinIdChanged(binId);
    }
}

int ProjectBinManager::totalAssetsCount() const {
    return static_cast<int>(m_db.getAllAssets().size());
}

int ProjectBinManager::createBin(const QString& name, int parentBinId, const QString& colorTag) {
    return m_db.createBin(name.trimmed().isEmpty() ? "New Bin" : name.trimmed(), parentBinId, colorTag);
}

bool ProjectBinManager::renameBin(int binId, const QString& newName) {
    return m_db.renameBin(binId, newName.trimmed());
}

bool ProjectBinManager::deleteBin(int binId) {
    bool ok = m_db.deleteBin(binId);
    if (ok && m_selectedBinId == binId) {
        setSelectedBinId(1); // fallback to Master
    }
    return ok;
}

int ProjectBinManager::importMediaFile(const QString& filePath, int binId) {
    QString cleanPath = filePath;
    if (cleanPath.startsWith("file://")) {
        cleanPath = QUrl(cleanPath).toLocalFile();
    }

    int targetBin = binId > 0 ? binId : m_selectedBinId;
    int assetId = m_db.importMediaFile(cleanPath, targetBin, m_appDataDir);
    if (assetId > 0) {
        ProjectAsset asset = m_db.getAssetById(assetId);
        emit mediaImported(assetId, asset.name);
    }
    return assetId;
}

bool ProjectBinManager::removeAsset(int assetId) {
    return m_db.removeAsset(assetId);
}

bool ProjectBinManager::moveAssetToBin(int assetId, int targetBinId) {
    return m_db.moveAssetToBin(assetId, targetBinId);
}

void ProjectBinManager::setSearchQuery(const QString& query) {
    if (m_assetModel) {
        m_assetModel->setSearchQuery(query);
    }
}

} // namespace ssa::project
