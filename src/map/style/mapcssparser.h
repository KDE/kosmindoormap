/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORMAP_MAPCSSPARSER_H
#define KOSMINDOORMAP_MAPCSSPARSER_H

#include "kosmindoormap_export.h"

#include <memory>

class QString;

namespace KOSMIndoorMap {

class MapCSSParserPrivate;
class MapCSSStyle;
class MapCSSRule;

/** MapCSS parser. */
class KOSMINDOORMAP_EXPORT MapCSSParser
{
public:
    explicit MapCSSParser();
    ~MapCSSParser();

    MapCSSStyle parse(const QString &fileName);

    [[nodiscard]] bool hasError() const;
    [[nodiscard]] QString fileName() const;
    [[nodiscard]] QString errorMessage() const;

private:
    friend class MapCSSParserPrivate;
    std::unique_ptr<MapCSSParserPrivate> d;
};

}

#endif // KOSMINDOORMAP_MAPCSSPARSER_H
