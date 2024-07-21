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

    /** @p oh evaluates to closed for the entire selected time range. */
    [[nodiscard]] bool isEntirelyClosedInRange(OSM::Element elem, const QByteArray &oh);

    /** @p oh is active at the current time (clamped to the selected time range). */
    [[nodiscard]] bool isAtCurrentTime(OSM::Element elem, const QByteArray &oh);

private:
    /** Current time clamped to selected time range. */
    [[nodiscard]] QDateTime currentDateTime() const;

    enum Result {
        UnknownResult = 0,
        HasEntireRangeResult = 1,
        EntirelyClosedInTimeRange = 2,
        HasCurrentTimeResult = 4,
        IsAtCurrentTime = 8,
    };
    Q_DECLARE_FLAGS(Results, Result)

    struct Entry {
        OSM::Id elementId;
        QByteArray oh;
        Results results;

        [[nodiscard]] bool operator<(const Entry &other) const;
    };

    std::vector<Entry> m_cacheEntries;

    QDateTime m_begin;
    QDateTime m_end;

    MapData m_mapData;
};

}

#endif // KOSMINDOORMAP_OPENINGHOURSCACHE_P_H
