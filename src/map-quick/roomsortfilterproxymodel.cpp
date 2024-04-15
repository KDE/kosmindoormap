/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "roomsortfilterproxymodel.h"
#include "roommodel.h"

using namespace KOSMIndoorMap;

RoomSortFilterProxyModel::RoomSortFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
    , m_collator(QLocale())
{
    setDynamicSortFilter(true);

    m_collator.setCaseSensitivity(Qt::CaseInsensitive);
    m_collator.setIgnorePunctuation(true);

    connect(this, &QAbstractProxyModel::sourceModelChanged, this, [this]() { sort(0); });
    connect(this, &RoomSortFilterProxyModel::filterStringChanged, this, &QSortFilterProxyModel::invalidate);
}

RoomSortFilterProxyModel::~RoomSortFilterProxyModel() = default;

bool RoomSortFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if (m_filter.isEmpty()) {
        return true;
    }

    // TODO building name only makes sense if there is more than one
    const auto idx = sourceModel()->index(source_row, 0, source_parent);
    return filterMatches(idx.data(RoomModel::NameRole).toString())
        || filterMatches(idx.data(RoomModel::NumberRole).toString())
        || filterMatches(idx.data(RoomModel::TypeNameRole).toString())
        || filterMatches(idx.data(RoomModel::BuildingNameRole).toString())
        || filterMatches(idx.data(RoomModel::LevelLongNameRole).toString());
}

bool RoomSortFilterProxyModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    const auto lhsBldg = source_left.data(RoomModel::BuildingNameRole).toString();
    const auto rhsBldg = source_right.data(RoomModel::BuildingNameRole).toString();
    if (lhsBldg == rhsBldg) {
        const int lhsLvl = source_left.data(RoomModel::LevelRole).toInt() / 10;
        const int rhsLvl = source_right.data(RoomModel::LevelRole).toInt() / 10;
        if (lhsLvl == rhsLvl) {
            auto lhsTitle = source_left.data(RoomModel::NumberRole).toString();
            if (lhsTitle.isEmpty()) {
                lhsTitle = source_left.data(RoomModel::NameRole).toString();
            }
            auto rhsTitle = source_right.data(RoomModel::NumberRole).toString();
            if (rhsTitle.isEmpty()) {
                rhsTitle = source_right.data(RoomModel::NameRole).toString();
            }
            return m_collator.compare(lhsTitle, rhsTitle) < 0;
        }
        return lhsLvl > rhsLvl;
    }
    return m_collator.compare(lhsBldg, rhsBldg) < 0;
}

bool RoomSortFilterProxyModel::filterMatches(const QString &s) const
{
    return s.contains(m_filter, Qt::CaseInsensitive); // TODO ignore diacritics
}

#include "moc_roomsortfilterproxymodel.cpp"
