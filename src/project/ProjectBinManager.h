#pragma once

#include <QObject>
#include <QString>
#include <memory>
#include "project/AssetDatabase.h"
#include "project/ProjectBinModel.h"
#include "project/AssetListModel.h"

namespace ssa::project {

class ProjectBinManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(ProjectBinModel* binModel READ binModel NOTIFY modelsChanged)
    Q_PROPERTY(AssetListModel* assetModel READ assetModel NOTIFY modelsChanged)
    Q_PROPERTY(int selectedBinId READ selectedBinId WRITE setSelectedBinId NOTIFY selectedBinIdChanged)
    Q_PROPERTY(int totalAssetsCount READ totalAssetsCount NOTIFY statsChanged)

public:
    explicit ProjectBinManager(QObject* parent = nullptr);
    ~ProjectBinManager() override = default;

    bool initialize(const QString& dbPath, const QString& appDataDir);

    ProjectBinModel* binModel() const { return m_binModel.get(); }
    AssetListModel* assetModel() const { return m_assetModel.get(); }

    int selectedBinId() const { return m_selectedBinId; }
    void setSelectedBinId(int binId);

    int totalAssetsCount() const;

    Q_INVOKABLE int createBin(const QString& name, int parentBinId = 1, const QString& colorTag = "#4f46e5");
    Q_INVOKABLE bool renameBin(int binId, const QString& newName);
    Q_INVOKABLE bool deleteBin(int binId);
    Q_INVOKABLE int importMediaFile(const QString& filePath, int binId = 1);
    Q_INVOKABLE bool removeAsset(int assetId);
    Q_INVOKABLE bool moveAssetToBin(int assetId, int targetBinId);
    Q_INVOKABLE void setSearchQuery(const QString& query);

signals:
    void modelsChanged();
    void selectedBinIdChanged(int binId);
    void statsChanged();
    void mediaImported(int assetId, const QString& name);

private:
    AssetDatabase m_db;
    std::unique_ptr<ProjectBinModel> m_binModel;
    std::unique_ptr<AssetListModel> m_assetModel;
    int m_selectedBinId = 1; // Master bin default
    QString m_appDataDir;
};

} // namespace ssa::project
