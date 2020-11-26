/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "config-kosmindoormap.h"
#include "openinghourscache_p.h"
#include "logging.h"

#ifdef HAVE_KOPENINGHOURS
#include <KOpeningHours/Interval>
#include <KOpeningHours/OpeningHours>
#endif

using namespace KOSMIndoorMap;

void OpeningHoursCache::setTimeRange(const QDateTime &begin, const QDateTime &end)
{
    if ((begin == m_begin || begin < QDateTime::currentDateTime()) && end == m_end) {
        return;
    }

    m_begin = std::max(begin, QDateTime::currentDateTime());
    m_end = end;
    m_cacheEntries.clear();
}

bool OpeningHoursCache::isClosed(OSM::Element elem, const QByteArray &oh)
{
#ifndef HAVE_KOPENINGHOURS
    Q_UNUSED(elem);
    Q_UNUSED(oh);
    return false;
#else
    const auto key = elem.id();
    const auto it = std::lower_bound(m_cacheEntries.begin(), m_cacheEntries.end(), key, [](auto lhs, auto rhs) {
        return lhs.key < rhs;
    });
    if (it != m_cacheEntries.end() && (*it).key == key) {
        return (*it).closed;
    }

    bool closed = false;

    KOpeningHours::OpeningHours expr(oh, KOpeningHours::OpeningHours::IntervalMode);
    expr.setLocation(elem.center().latF(), elem.center().lonF());
    // TODO holiday region, timezone

    if (expr.error() != KOpeningHours::OpeningHours::NoError) {
        qCDebug(Log) << "opening hours expression error:" << expr.error() << oh;
    } else {
        auto i = expr.interval(m_begin);
        while (i.state() == KOpeningHours::Interval::Closed && i.end().isValid() && i.end() < m_end) {
            i = expr.nextInterval(i);
        }
        closed = i.state() == KOpeningHours::Interval::Closed;
    }
    m_cacheEntries.insert(it, {key, closed});
    return closed;
#endif
}
