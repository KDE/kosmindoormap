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

bool OpeningHoursCache::isEntirelyClosedInRange(OSM::Element elem, const QByteArray &oh)
{
    Entry entry{elem.id(), oh, UnknownResult};
    const auto it = std::lower_bound(m_cacheEntries.begin(), m_cacheEntries.end(), entry);
    if (it != m_cacheEntries.end() && (*it).elementId == elem.id() && (*it).oh == oh) {
        if ((*it).results & HasEntireRangeResult) {
            return (*it).results & EntirelyClosedInTimeRange;
        }
        entry.results = (*it).results;
    }

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
        if (i.state() == KOpeningHours::Interval::Closed) {
            entry.results |= EntirelyClosedInTimeRange;
        }
    }
    entry.results |= HasEntireRangeResult;
    const bool closed = entry.results & EntirelyClosedInTimeRange;

    if (it != m_cacheEntries.end() && (*it).elementId == elem.id() && (*it).oh == oh) {
        *it = std::move(entry);
    } else {
        m_cacheEntries.insert(it, std::move(entry));
    }
    return closed;
}

bool OpeningHoursCache::isAtCurrentTime(OSM::Element elem, const QByteArray &oh)
{
    Entry entry{elem.id(), oh, UnknownResult};
    const auto it = std::lower_bound(m_cacheEntries.begin(), m_cacheEntries.end(), entry);
    if (it != m_cacheEntries.end() && (*it).elementId == elem.id() && (*it).oh == oh) {
        if ((*it).results & HasCurrentTimeResult) {
            return (*it).results & IsAtCurrentTime;
        }
        entry.results = (*it).results;
    }

    KOpeningHours::OpeningHours expr(oh, KOpeningHours::OpeningHours::IntervalMode);
    expr.setLocation(elem.center().latF(), elem.center().lonF());
    expr.setRegion(m_mapData.regionCode());
    expr.setTimeZone(m_mapData.timeZone());

    if (expr.error() != KOpeningHours::OpeningHours::NoError) {
        qCDebug(Log) << "opening hours expression error:" << expr.error() << oh << elem.url();
    } else {
        const auto i = expr.interval(currentDateTime());
        if (i.state() == KOpeningHours::Interval::Open) {
            entry.results |= IsAtCurrentTime;
        }
    }

    entry.results |= HasCurrentTimeResult;
    const bool open = entry.results & IsAtCurrentTime;

    if (it != m_cacheEntries.end() && (*it).elementId == elem.id() && (*it).oh == oh) {
        *it = std::move(entry);
    } else {
        m_cacheEntries.insert(it, std::move(entry));
    }
    return open;
}

QDateTime OpeningHoursCache::currentDateTime() const
{
    if (!m_begin.isValid() && !m_end.isValid()) {
        return QDateTime::currentDateTime();
    }
    if (!m_begin.isValid()) {
        return std::min(m_end.addSecs(-1), QDateTime::currentDateTime());
    }
    if (!m_end.isValid()) {
        return std::max(m_begin, QDateTime::currentDateTime());
    }
    return std::clamp(QDateTime::currentDateTime(), m_begin, m_end.addSecs(-1));
}
