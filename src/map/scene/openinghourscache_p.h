/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORMAP_OPENINGHOURSCACHE_P_H
#define KOSMINDOORMAP_OPENINGHOURSCACHE_P_H

#include <KOSMIndoorMap/MapData>

#include <KOSM/Element>

#include <QDateTime>

#include <vector>

namespace KOSMIndoorMap {

/** Opening hours expression evaluation cache for the currently displayed time range. */
class OpeningHoursCache
{
public:
    OpeningHoursCache();
    ~OpeningHoursCache();
    OpeningHoursCache(const OpeningHoursCache&) = delete;
    OpeningHoursCache& operator=(const OpeningHoursCache&) = delete;

    void setMapData(const MapData &mapData);
    void setTimeRange(const QDateTime &begin, const QDateTime &end);

    bool isClosed(OSM::Element elem, const QByteArray &oh);

private:
    struct Entry {
        OSM::Id key;
        bool closed;
    };
    std::vector<Entry> m_cacheEntries;

    QDateTime m_begin;
    QDateTime m_end;

    MapData m_mapData;
};

}

#endif // KOSMINDOORMAP_OPENINGHOURSCACHE_P_H
