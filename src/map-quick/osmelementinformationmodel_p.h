/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORMAP_OSMELEMENTINFORMATIONMODEL_P_H
#define KOSMINDOORMAP_OSMELEMENTINFORMATIONMODEL_P_H

#include <KLocalizedString>
#include <KLazyLocalizedString>

#include <QCoreApplication>
#include <QLocale>

#include <cstring>
#include <iterator>

namespace KOSMIndoorMap {

struct ValueMapEntry
{
    const char *keyName;
    const KLazyLocalizedString label;
};

template <typename MapEntry, std::size_t N>
inline constexpr bool isSortedLookupTable(const MapEntry(&map)[N])
{
#if __cplusplus > 201703L
    return std::is_sorted(std::begin(map), std::end(map), [](const auto &lhs, const auto &rhs) {
        return std::strcmp(lhs.keyName, rhs.keyName) < 0;
    });
#else
    Q_UNUSED(map);
    return true;
#endif
}

enum TranslationOption {
    ReturnUnknownKey,
    ReturnEmptyOnUnknownKey
};

template <typename MapEntry, std::size_t N>
inline QString translateValue(const char *keyName, const MapEntry(&map)[N], TranslationOption opt = ReturnUnknownKey)
{
    const auto it = std::lower_bound(std::begin(map), std::end(map), keyName, [](const auto &lhs, auto rhs) {
        return std::strcmp(lhs.keyName, rhs) < 0;
    });
    if (it == std::end(map) || std::strcmp((*it).keyName, keyName) != 0) {
        return opt == ReturnUnknownKey ? QString::fromUtf8(keyName) : QString();
    }

    return (*it).label.toString();
}

template <typename MapEntry, std::size_t N>
inline QString translateValues(const QByteArray &values, const MapEntry(&map)[N])
{
    const auto l = values.split(';');
    QStringList out;
    out.reserve(l.size());
    for (const auto &value : l) {
        const auto s = translateValue(value.constData(), map);
        if (!s.isEmpty()) {
            out.push_back(s);
        }
    }
    std::sort(out.begin(), out.end());
    out.erase(std::unique(out.begin(), out.end()), out.end());
    return QLocale().createSeparatedList(out);
}

}

#endif
