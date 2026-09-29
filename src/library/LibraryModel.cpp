#include "LibraryModel.h"

namespace ssa::library {

LibraryModel::LibraryModel(LibraryManager* manager, QObject* parent)
    : QAbstractListModel(parent), m_manager(manager) 
{
    if (m_manager) {
        connect(m_manager, &LibraryManager::libraryUpdated, this, &LibraryModel::reloadData);
        reloadData();
    }
}

int LibraryModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return m_items.size();
}

QVariant LibraryModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= m_items.size()) return QVariant();

    const MediaItem& item = m_items[index.row()];

    switch (role) {
    case IdRole: return item.id;
    case TypeRole: return (item.type == MediaType::Recording) ? "recording" : "screenshot";
    case PathRole: return item.path;
    case TimestampRole: return QVariant::fromValue(static_cast<qulonglong>(item.timestamp));
    case DurationRole: return item.durationMs;
    case ResolutionRole: return item.resolution;
    case ThumbnailPathRole: return item.thumbnailPath;
    case TagsRole: return item.tags;
    default: return QVariant();
    }
}

QHash<int, QByteArray> LibraryModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[TypeRole] = "type";
    roles[PathRole] = "path";
    roles[TimestampRole] = "timestamp";
    roles[DurationRole] = "durationMs";
    roles[ResolutionRole] = "resolution";
    roles[ThumbnailPathRole] = "thumbnailPath";
    roles[TagsRole] = "tags";
    return roles;
}

void LibraryModel::reloadData() {
    beginResetModel();
    if (m_manager) {
        m_items = m_manager->getAllItems();
    }
    endResetModel();
}

} // namespace ssa::library
