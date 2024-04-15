/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORMAP_ROOMSORTFILTERMODEL_H
#define KOSMINDOORMAP_ROOMSORTFILTERMODEL_H

#include <QCollator>
#include <QSortFilterProxyModel>

namespace KOSMIndoorMap {

/** Filtering/sorting on top of the RoomModel.
 *  - filters on all visible roles
 *  - sorts while keeping the grouping intact
 */
class RoomSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QString filterString MEMBER m_filter NOTIFY filterStringChanged)

public:
    explicit RoomSortFilterProxyModel(QObject *parent = nullptr);
    ~RoomSortFilterProxyModel();

Q_SIGNALS:
    void filterStringChanged();

protected:
    [[nodiscard]] bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
    [[nodiscard]] bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;

private:
    [[nodiscard]] bool filterMatches(const QString &s) const;

    QCollator m_collator;
    QString m_filter;
};

}

#endif
