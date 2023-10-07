/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef OSM_LANGUAGES_H
#define OSM_LANGUAGES_H

#include "kosm_export.h"

#include <string>
#include <vector>

class QLocale;

namespace OSM {

/** Languages in preference order to consider when looking up
 *  translated tag values.
 *  @see https://wiki.openstreetmap.org/wiki/Multilingual_names
 *  @see https://wiki.openstreetmap.org/wiki/Key:name#Variants
 *  @see https://wiki.openstreetmap.org/wiki/Names#Localization
 */
class Languages
{
public:
    explicit Languages() = default;
    Languages(Languages&&) noexcept = default;
    Languages(const Languages&) = delete;
    ~Languages() = default;
    Languages& operator=(Languages&&) = default;
    Languages& operator=(const Languages&) = delete;

    /** Convert QLocale::uiLanguages() into an OSM::Languages set.
     *  The result should be cached in case of repeated use,
     *  QLocale::uiLanguages() is relatively expensive.
     */
    KOSM_EXPORT static Languages fromQLocale(const QLocale &locale);

    std::vector<std::string> languages;
};

}

#endif
