/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORMAP_LOCALIZATION_P_H
#define KOSMINDOORMAP_LOCALIZATION_P_H

#include "localization.h"

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

template <typename MapEntry, std::size_t N>
inline QString translateValue(const char *keyName, const MapEntry(&map)[N], Localization::TranslationOption opt = Localization::ReturnUnknownKey)
{
    const auto it = std::lower_bound(std::begin(map), std::end(map), keyName, [](const auto &lhs, auto rhs) {
        return std::strcmp(lhs.keyName, rhs) < 0;
    });
    if (it == std::end(map) || std::strcmp((*it).keyName, keyName) != 0) {
        return opt == Localization::ReturnUnknownKey ? QString::fromUtf8(keyName) : QString();
    }

    return (*it).label.toString();
}

template <typename MapEntry, std::size_t N>
inline bool hasTranslatedValue(const char *value, const MapEntry(&map)[N])
{
    const auto it = std::lower_bound(std::begin(map), std::end(map), value, [](const auto &lhs, auto rhs) {
        return std::strcmp(lhs.keyName, rhs) < 0;
    });
    return it != std::end(map) && std::strcmp((*it).keyName, value) == 0;
}

template <typename MapEntry, std::size_t N>
inline QString translateValues(const QByteArray &values, const MapEntry(&map)[N], Localization::TranslationOption opt = Localization::ReturnUnknownKey)
{
    const auto l = values.split(';');
    QStringList out;
    out.reserve(l.size());
    for (const auto &value : l) {
        const auto s = translateValue(value.constData(), map, opt);
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
