#pragma once

#include <QSortFilterProxyModel>
#include "library/LibraryModel.h"

namespace ssa::library {

class LibraryFilterModel : public QSortFilterProxyModel {
    Q_OBJECT
    Q_PROPERTY(QString searchQuery READ searchQuery WRITE setSearchQuery NOTIFY searchQueryChanged)
    Q_PROPERTY(QString typeFilter READ typeFilter WRITE setTypeFilter NOTIFY typeFilterChanged)

public:
    explicit LibraryFilterModel(QObject* parent = nullptr);

    QString searchQuery() const;
    void setSearchQuery(const QString& query);

    QString typeFilter() const;
    void setTypeFilter(const QString& type);

signals:
    void searchQueryChanged();
    void typeFilterChanged();

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

private:
    QString m_searchQuery;
    QString m_typeFilter; // e.g., "recording", "screenshot", or "" for all
};

} // namespace ssa::library
