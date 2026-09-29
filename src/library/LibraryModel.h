#pragma once

#include <QAbstractListModel>
#include "library/LibraryManager.h"

namespace ssa::library {

class LibraryModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum MediaRoles {
        IdRole = Qt::UserRole + 1,
        TypeRole,
        PathRole,
        TimestampRole,
        DurationRole,
        ResolutionRole,
        ThumbnailPathRole,
        TagsRole
    };

    explicit LibraryModel(LibraryManager* manager, QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

public slots:
    void reloadData();

private:
    LibraryManager* m_manager;
    QList<MediaItem> m_items;
};

} // namespace ssa::library
