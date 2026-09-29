#include "LibraryFilterModel.h"

namespace ssa::library {

LibraryFilterModel::LibraryFilterModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
    setFilterCaseSensitivity(Qt::CaseInsensitive);
}

QString LibraryFilterModel::searchQuery() const {
    return m_searchQuery;
}

void LibraryFilterModel::setSearchQuery(const QString& query) {
    if (m_searchQuery != query) {
        m_searchQuery = query;
        emit searchQueryChanged();
        invalidateFilter();
    }
}

QString LibraryFilterModel::typeFilter() const {
    return m_typeFilter;
}

void LibraryFilterModel::setTypeFilter(const QString& type) {
    if (m_typeFilter != type) {
        m_typeFilter = type;
        emit typeFilterChanged();
        invalidateFilter();
    }
}

bool LibraryFilterModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const {
    QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
    if (!index.isValid()) return false;

    QString type = sourceModel()->data(index, LibraryModel::TypeRole).toString();
    QString path = sourceModel()->data(index, LibraryModel::PathRole).toString();
    QString tags = sourceModel()->data(index, LibraryModel::TagsRole).toString();

    if (!m_typeFilter.isEmpty() && type != m_typeFilter) {
        return false;
    }

    if (!m_searchQuery.isEmpty()) {
        bool matchesPath = path.contains(m_searchQuery, Qt::CaseInsensitive);
        bool matchesTags = tags.contains(m_searchQuery, Qt::CaseInsensitive);
        if (!matchesPath && !matchesTags) {
            return false;
        }
    }

    return true;
}

} // namespace ssa::library
