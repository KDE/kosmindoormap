/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "openinghourscache_p.h"
#include "logging.h"

#include <KOpeningHours/Interval>
#include <KOpeningHours/OpeningHours>

#include <QTimeZone>

using namespace KOSMIndoorMap;

bool OpeningHoursCache::Entry::operator<(const Entry &other) const
{
    if (elementId == other.elementId) {
        return oh < other.oh;
    }
    return elementId < other.elementId;
}

OpeningHoursCache::OpeningHoursCache()
{
    setTimeRange({}, {});
}

OpeningHoursCache::~OpeningHoursCache() = default;

void OpeningHoursCache::setMapData(const MapData &mapData)
{
    if (m_mapData == mapData) {
        return;
    }
    m_mapData = mapData;
    m_cacheEntries.clear();
}

void OpeningHoursCache::setTimeRange(const QDateTime &begin, const QDateTime &end)
{
    const auto actualBegin = begin.isValid() ? begin : QDateTime::currentDateTime();
    const auto actualEnd = (end.isValid() && end > actualBegin) ? end : actualBegin.addYears(1);

    if (actualBegin == m_begin && actualEnd == m_end) {
        return;
    }

    m_begin = actualBegin;
    m_end = actualEnd;
    m_cacheEntries.clear();
}

bool OpeningHoursCache::isClosed(OSM::Element elem, const QByteArray &oh)
{
    Entry entry{elem.id(), oh, false};
    const auto it = std::lower_bound(m_cacheEntries.begin(), m_cacheEntries.end(), entry);
    if (it != m_cacheEntries.end() && (*it).elementId == elem.id() && (*it).oh == oh) {
        return (*it).closed;
    }

    bool closed = false;

    KOpeningHours::OpeningHours expr(oh, KOpeningHours::OpeningHours::IntervalMode);
    expr.setLocation(elem.center().latF(), elem.center().lonF());
    expr.setRegion(m_mapData.regionCode());
    expr.setTimeZone(m_mapData.timeZone());

    if (expr.error() != KOpeningHours::OpeningHours::NoError) {
        qCDebug(Log) << "opening hours expression error:" << expr.error() << oh << elem.url();
    } else {
        auto i = expr.interval(m_begin);
        while (i.state() == KOpeningHours::Interval::Closed && !i.hasOpenEnd() && (i.end() < m_end || !m_end.isValid())) {
            i = expr.nextInterval(i);
        }
        if (expr.error() != KOpeningHours::OpeningHours::NoError) {
            qCDebug(Log) << "opening hours expression runtime error:" << expr.error() << oh << i << elem.url();
        }
        closed = entry.closed = i.state() == KOpeningHours::Interval::Closed;
    }
    m_cacheEntries.insert(it, std::move(entry));
    return closed;
}
