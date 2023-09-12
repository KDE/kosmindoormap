/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "amenitysortfilterproxymodel.h"
#include "amenitymodel.h"

using namespace KOSMIndoorMap;

AmenitySortFilterProxyModel::AmenitySortFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
    , m_collator(QLocale())
{
    setDynamicSortFilter(true);

    m_collator.setCaseSensitivity(Qt::CaseInsensitive);
    m_collator.setIgnorePunctuation(true);

    connect(this, &QAbstractProxyModel::sourceModelChanged, this, [this]() { sort(0); });
    connect(this, &AmenitySortFilterProxyModel::filterStringChanged, this, &QSortFilterProxyModel::invalidate);
}

AmenitySortFilterProxyModel::~AmenitySortFilterProxyModel() = default;


bool AmenitySortFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if (m_filter.isEmpty()) {
        return true;
    }

    const auto idx = sourceModel()->index(source_row, 0, source_parent);
    return filterMatches(idx.data(AmenityModel::NameRole).toString())
        || filterMatches(idx.data(AmenityModel::TypeNameRole).toString())
        || filterMatches(idx.data(AmenityModel::GroupNameRole).toString())
        || filterMatches(idx.data(AmenityModel::FallbackNameRole).toString())
        || filterMatches(idx.data(AmenityModel::CuisineRole).toString());
}

bool AmenitySortFilterProxyModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    const auto lhsGroup = source_left.data(AmenityModel::GroupRole).toInt();
    const auto rhsGroup = source_right.data(AmenityModel::GroupRole).toInt();
    if (lhsGroup == rhsGroup) {
        auto lhsTitle = source_left.data(AmenityModel::NameRole).toString();
        if (lhsTitle.isEmpty()) {
            lhsTitle = source_left.data(AmenityModel::TypeNameRole).toString();
        }
        auto rhsTitle = source_right.data(AmenityModel::NameRole).toString();
        if (rhsTitle.isEmpty()) {
            rhsTitle = source_right.data(AmenityModel::TypeNameRole).toString();
        }
        return m_collator.compare(lhsTitle, rhsTitle) < 0;
    }
    return lhsGroup < rhsGroup;
}

bool AmenitySortFilterProxyModel::filterMatches(const QString &s) const
{
    return s.contains(m_filter, Qt::CaseInsensitive); // TODO ignore diacritics
}
