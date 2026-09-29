#include "project/AssetListModel.h"
#include <QTime>

namespace ssa::project {

AssetListModel::AssetListModel(AssetDatabase* db, QObject* parent)
    : QAbstractListModel(parent)
    , m_db(db)
{
    if (m_db) {
        connect(m_db, &AssetDatabase::databaseUpdated, this, &AssetListModel::reloadData);
        reloadData();
    }
}

int AssetListModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return static_cast<int>(m_assets.size());
}

QVariant AssetListModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= m_assets.size()) {
        return QVariant();
    }

    const ProjectAsset& asset = m_assets.at(index.row());

    switch (role) {
    case AssetIdRole:
        return asset.assetId;
    case BinIdRole:
        return asset.binId;
    case NameRole:
        return asset.name;
    case FilePathRole:
        return asset.filePath;
    case MediaTypeRole:
        return asset.mediaType;
    case DurationMsRole:
        return static_cast<qint64>(asset.durationMs);
    case WidthRole:
        return asset.width;
    case HeightRole:
        return asset.height;
    case FpsRole:
        return asset.fps;
    case SampleRateRole:
        return asset.sampleRate;
    case ChannelsRole:
        return asset.channels;
    case ThumbnailPathRole:
        return asset.thumbnailPath;
    case FileSizeBytesRole:
        return static_cast<qint64>(asset.fileSizeBytes);
    case CreatedAtRole:
        return static_cast<qint64>(asset.createdAt);
    case FormattedDurationRole:
        return formatDuration(asset.durationMs);
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> AssetListModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[AssetIdRole] = "assetId";
    roles[BinIdRole] = "binId";
    roles[NameRole] = "name";
    roles[FilePathRole] = "filePath";
    roles[MediaTypeRole] = "mediaType";
    roles[DurationMsRole] = "durationMs";
    roles[WidthRole] = "width";
    roles[HeightRole] = "height";
    roles[FpsRole] = "fps";
    roles[SampleRateRole] = "sampleRate";
    roles[ChannelsRole] = "channels";
    roles[ThumbnailPathRole] = "thumbnailPath";
    roles[FileSizeBytesRole] = "fileSizeBytes";
    roles[CreatedAtRole] = "createdAt";
    roles[FormattedDurationRole] = "formattedDuration";
    return roles;
}

void AssetListModel::setCurrentBinId(int binId) {
    if (m_currentBinId != binId) {
        m_currentBinId = binId;
        emit currentBinIdChanged(binId);
        reloadData();
    }
}

void AssetListModel::setSearchQuery(const QString& query) {
    if (m_searchQuery != query) {
        m_searchQuery = query;
        emit searchQueryChanged(query);
        reloadData();
    }
}

void AssetListModel::reloadData() {
    beginResetModel();
    if (!m_db) {
        m_assets.clear();
        endResetModel();
        return;
    }

    if (!m_searchQuery.trimmed().isEmpty()) {
        m_assets = m_db->searchAssets(m_searchQuery.trimmed());
    } else if (m_currentBinId == -1) {
        m_assets = m_db->getAllAssets();
    } else {
        m_assets = m_db->getAssetsInBin(m_currentBinId);
    }
    endResetModel();
}

QString AssetListModel::formatDuration(int64_t ms) const {
    if (ms <= 0) return "00:00";
    int totalSecs = static_cast<int>(ms / 1000);
    int mins = totalSecs / 60;
    int secs = totalSecs % 60;
    int hrs = mins / 60;
    mins = mins % 60;

    if (hrs > 0) {
        return QString("%1:%2:%3")
            .arg(hrs, 2, 10, QChar('0'))
            .arg(mins, 2, 10, QChar('0'))
            .arg(secs, 2, 10, QChar('0'));
    } else {
        return QString("%1:%2")
            .arg(mins, 2, 10, QChar('0'))
            .arg(secs, 2, 10, QChar('0'));
    }
}

} // namespace ssa::project
