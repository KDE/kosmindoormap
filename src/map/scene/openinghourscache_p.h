/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORMAP_OPENINGHOURSCACHE_P_H
#define KOSMINDOORMAP_OPENINGHOURSCACHE_P_H

#include <KOSM/Element>

#include <QDateTime>

#include <vector>

namespace KOSMIndoorMap {

/** Opening hours expression evaluation cache for the currently displayed time range. */
class OpeningHoursCache
{
public:
    OpeningHoursCache() = default;
    ~OpeningHoursCache() = default;
    OpeningHoursCache(const OpeningHoursCache&) = delete;
    OpeningHoursCache& operator=(const OpeningHoursCache&) = delete;

    void setTimeRange(const QDateTime &begin, const QDateTime &end);

    bool isClosed(OSM::Element elem, const QByteArray &oh);

private:
    struct Entry {
        OSM::Id key;
        bool closed;
    };

    QDateTime m_begin = QDateTime::currentDateTime();
    QDateTime m_end;
    std::vector<Entry> m_cacheEntries;
};

}

#endif // KOSMINDOORMAP_OPENINGHOURSCACHE_P_H
