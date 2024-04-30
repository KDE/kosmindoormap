/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORMAP_MAPCSSPARSER_P_H
#define KOSMINDOORMAP_MAPCSSPARSER_P_H

#include "mapcssparser.h"
#include "mapcssparsercontext_p.h"
#include "mapcsstypes.h"

#include <QString>

namespace KOSMIndoorMap {

class MapCSSStyle;
class MapCSSRule;

class MapCSSParserPrivate : public MapCSSParserContext
{
public:
    MapCSSParserPrivate() : MapCSSParserContext(ParseMapCSS) {};

    void parse(MapCSSStyle *style, const QString &fileName, ClassSelectorKey importClass);

    [[nodiscard]] inline static MapCSSParserPrivate* get(MapCSSParser *parser) { return parser->d.get(); }
};

}

#endif
