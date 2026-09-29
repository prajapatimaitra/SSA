#include "project/ProjectBinModel.h"

namespace ssa::project {

ProjectBinModel::ProjectBinModel(AssetDatabase* db, QObject* parent)
    : QAbstractListModel(parent)
    , m_db(db)
{
    if (m_db) {
        connect(m_db, &AssetDatabase::databaseUpdated, this, &ProjectBinModel::reloadData);
        reloadData();
    }
}

int ProjectBinModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return static_cast<int>(m_bins.size());
}

QVariant ProjectBinModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= m_bins.size()) {
        return QVariant();
    }

    const ProjectBin& bin = m_bins.at(index.row());

    switch (role) {
    case BinIdRole:
        return bin.binId;
    case ParentBinIdRole:
        return bin.parentBinId;
    case NameRole:
        return bin.name;
    case ColorTagRole:
        return bin.colorTag;
    case CreatedAtRole:
        return static_cast<qint64>(bin.createdAt);
    case DepthRole:
        return m_binDepths.value(bin.binId, 0);
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> ProjectBinModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[BinIdRole] = "binId";
    roles[ParentBinIdRole] = "parentBinId";
    roles[NameRole] = "name";
    roles[ColorTagRole] = "colorTag";
    roles[CreatedAtRole] = "createdAt";
    roles[DepthRole] = "depth";
    return roles;
}

void ProjectBinModel::reloadData() {
    beginResetModel();
    if (m_db) {
        m_bins = m_db->getAllBins();
        calculateDepths();
    } else {
        m_bins.clear();
        m_binDepths.clear();
    }
    endResetModel();
}

void ProjectBinModel::calculateDepths() {
    m_binDepths.clear();
    QMap<int, int> parentMap;
    for (const auto& bin : m_bins) {
        parentMap[bin.binId] = bin.parentBinId;
    }

    for (const auto& bin : m_bins) {
        int depth = 0;
        int currentParent = bin.parentBinId;
        while (currentParent > 0 && parentMap.contains(currentParent) && depth < 20) {
            depth++;
            currentParent = parentMap[currentParent];
        }
        m_binDepths[bin.binId] = depth;
    }
}

} // namespace ssa::project
