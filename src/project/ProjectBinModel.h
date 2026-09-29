#pragma once

#include <QAbstractListModel>
#include <QMap>
#include "project/AssetDatabase.h"

namespace ssa::project {

class ProjectBinModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum BinRoles {
        BinIdRole = Qt::UserRole + 1,
        ParentBinIdRole,
        NameRole,
        ColorTagRole,
        CreatedAtRole,
        DepthRole
    };

    explicit ProjectBinModel(AssetDatabase* db, QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

public slots:
    void reloadData();

private:
    AssetDatabase* m_db;
    QList<ProjectBin> m_bins;
    QMap<int, int> m_binDepths;

    void calculateDepths();
};

} // namespace ssa::project
