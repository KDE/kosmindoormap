/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "localization.h"
#include "localization_data.cpp"

#include <KOSM/Element>

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

QString Localization::genderSegregation(OSM::Element element)
{
    QStringList l;
    for (const auto &gender : gender_type_map) {
        const auto v = element.tagValue(gender.keyName);
        if (v.isEmpty() || v == "no") {
            continue;
        }
        l.push_back(gender.label.toString());
    }
    return QLocale().createSeparatedList(l);
}

bool Localization::hasGenderSegregrationKey(OSM::Element element)
{
    return std::any_of(element.tagsBegin(), element.tagsEnd(), [](const auto &tag) {
        const auto it = std::lower_bound(std::begin(gender_type_map), std::end(gender_type_map), tag.key.name(), [](const auto &lhs, auto rhs) {
            return std::strcmp(lhs.keyName, rhs) < 0;
        });
        return it != std::end(gender_type_map) && std::strcmp((*it).keyName, tag.key.name()) == 0;
    });
}
