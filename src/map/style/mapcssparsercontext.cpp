/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mapcssparsercontext_p.h"

#include "mapcssdeclaration_p.h"
#include "mapcssparser.h"
#include "mapcssparser_p.h"
#include "mapcssrule_p.h"
#include "mapcssstyle_p.h"
#include "logging.h"

#include <QFileInfo>

using namespace KOSMIndoorMap;

bool MapCSSParserContext::addImport(char* fileName, ClassSelectorKey importClass)
{
    auto cssFile = QString::fromUtf8(fileName);
    free(fileName);

    if (QFileInfo(cssFile).isRelative()) {
        cssFile = QFileInfo(m_currentFileName).absolutePath() + QLatin1Char('/') + cssFile;
    }

    MapCSSParser p;
    MapCSSParserPrivate::get(&p)->parse(m_currentStyle, cssFile, importClass);
    if (p.hasError()) {
        m_error = true;
        m_errorMsg = p.errorMessage();
    }
    return !p.hasError();
}

void MapCSSParserContext::addRule(MapCSSRule *rule)
{
    if (!m_importClass.isNull()) {
        auto decl = new MapCSSDeclaration(MapCSSDeclaration::ClassDeclaration);
        decl->setClassSelectorKey(m_importClass);
        rule->addDeclaration(decl);
    }
    MapCSSStylePrivate::get(m_currentStyle)->m_rules.push_back(std::unique_ptr<MapCSSRule>(rule));
}

void MapCSSParserContext::setError(const QString &msg, int line, int column)
{
    m_error = true;
    m_errorMsg = msg;
    m_line = line;
    m_column = column;
}

ClassSelectorKey MapCSSParserContext::makeClassSelector(const char *str, std::size_t len) const
{
    return MapCSSStylePrivate::get(m_currentStyle)->m_classSelectorRegistry.makeKey(str, len, OSM::StringMemory::Transient);
}

LayerSelectorKey MapCSSParserContext::makeLayerSelector(const char *str, std::size_t len) const
{
    if (!str || std::strcmp(str, "default") == 0) {
        return {};
    }
    return MapCSSStylePrivate::get(m_currentStyle)->m_layerSelectorRegistry.makeKey(str, len, OSM::StringMemory::Transient);
}
