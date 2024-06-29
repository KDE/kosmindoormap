/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "localization.h"
#include "localization_data.cpp"

using namespace KOSMIndoorMap;

bool Localization::hasAmenityTypeTranslation(const char *value)
{
    return hasTranslatedValue(value, amenity_map);
}

QString Localization::amenityType(const char *value, Localization::TranslationOption opt)
{
    return translateValue(value, amenity_map, opt);
}

QString Localization::amenityTypes(const QByteArray &value, Localization::TranslationOption opt)
{
    const auto types = value.split(';');
    QStringList l;
    for (const auto &type : types) {
        auto s = Localization::amenityType(type.trimmed().constData(), opt);
        if (!s.isEmpty()) {
            l.push_back(std::move(s));
        }
    }
    return QLocale().createSeparatedList(l);
}

QString Localization::cuisineTypes(const QByteArray &value, Localization::TranslationOption opt)
{
    return translateValues(value, cuisine_map, opt);
}
