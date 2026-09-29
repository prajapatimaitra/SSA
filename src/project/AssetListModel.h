#pragma once

#include <QAbstractListModel>
#include "project/AssetDatabase.h"

namespace ssa::project {

class AssetListModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(int currentBinId READ currentBinId WRITE setCurrentBinId NOTIFY currentBinIdChanged)
    Q_PROPERTY(QString searchQuery READ searchQuery WRITE setSearchQuery NOTIFY searchQueryChanged)

public:
    enum AssetRoles {
        AssetIdRole = Qt::UserRole + 1,
        BinIdRole,
        NameRole,
        FilePathRole,
        MediaTypeRole,
        DurationMsRole,
        WidthRole,
        HeightRole,
        FpsRole,
        SampleRateRole,
        ChannelsRole,
        ThumbnailPathRole,
        FileSizeBytesRole,
        CreatedAtRole,
        FormattedDurationRole
    };

    explicit AssetListModel(AssetDatabase* db, QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    int currentBinId() const { return m_currentBinId; }
    void setCurrentBinId(int binId);

    QString searchQuery() const { return m_searchQuery; }
    void setSearchQuery(const QString& query);

signals:
    void currentBinIdChanged(int binId);
    void searchQueryChanged(const QString& query);

public slots:
    void reloadData();

private:
    AssetDatabase* m_db;
    int m_currentBinId = -1; // -1 means All Assets
    QString m_searchQuery;
    QList<ProjectAsset> m_assets;

    QString formatDuration(int64_t ms) const;
};

} // namespace ssa::project
