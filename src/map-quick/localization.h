/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORMAP_LOCALIZATION_H
#define KOSMINDOORMAP_LOCALIZATION_H

class QByteArray;
class QString;

namespace OSM {
class Element;
}

namespace KOSMIndoorMap {

/** Translations of various OSM tag values. */
namespace Localization
{

/** Control how unknown elements get translated. */
enum TranslationOption {
    ReturnUnknownKey,
    ReturnEmptyOnUnknownKey
};

/** Returns @c true if we can translate @p value. */
bool hasAmenityTypeTranslation(const char *value);

/** Translated name for an amenity tag value (after list splitting).
 *  This also covers values of similar/overlapping tags (office, tourism, leisure, historic, room, building, etc).
 */
QString amenityType(const char *value, Localization::TranslationOption opt = Localization::ReturnUnknownKey);

/** Translated list of amenity tag values (including list splitting).
 *  This also covers values of similar/overlapping tags (office, tourism, leisure, historic, room, building, etc).
 */
QString amenityTypes(const QByteArray &value, Localization::TranslationOption opt = Localization::ReturnUnknownKey);

/** Translated values of the cuisine tag (does list splitting). */
QString cuisineTypes(const QByteArray &value, Localization::TranslationOption opt = Localization::ReturnUnknownKey);

/** Translated gender segregation information e.g. for toilets. */
[[nodiscard]] QString genderSegregation(OSM::Element element);
/** Checks whether @p element contains a known key for gender segregation information. */
[[nodiscard]] bool hasGenderSegregrationKey(OSM::Element element);
}

}

#endif // KOSMINDOORMAP_LOCALIZATION_H
