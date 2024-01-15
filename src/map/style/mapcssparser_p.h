/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORMAP_MAPCSSPARSER_P_H
#define KOSMINDOORMAP_MAPCSSPARSER_P_H

#include "mapcsstypes.h"

#include <QString>

namespace KOSMIndoorMap {

class MapCSSStyle;
class MapCSSRule;

class MapCSSParserPrivate
{
public:
    void parse(MapCSSStyle *style, const QString &fileName, ClassSelectorKey importClass);

    /** @internal for use by the generated parser only. */
    [[nodiscard]] bool addImport(char *fileName, ClassSelectorKey importClass);
    void addRule(MapCSSRule *rule);
    void setError(const QString &msg, int line, int column);

    [[nodiscard]] ClassSelectorKey makeClassSelector(const char *str, std::size_t len) const;
    [[nodiscard]] LayerSelectorKey makeLayerSelector(const char *str, std::size_t len) const;

    MapCSSStyle *m_currentStyle = nullptr;
    QString m_currentFileName;
    ClassSelectorKey m_importClass;
    bool m_error = false;
    QString m_errorMsg;
    int m_line = 0;
    int m_column = 0;
};

}

#endif
