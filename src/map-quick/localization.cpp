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

QString Localization::cuisineTypes(const QByteArray &value, Localization::TranslationOption opt)
{
    return translateValues(value, cuisine_map, opt);
}
