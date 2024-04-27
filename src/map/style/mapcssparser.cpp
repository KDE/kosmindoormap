/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mapcssparser.h"
#include "mapcssparser_p.h"
#include "logging.h"

#include "mapcssparser_impl.h"
#include "mapcssdeclaration_p.h"
#include "mapcssrule_p.h"
#include "mapcssscanner.h"
#include "mapcssstyle.h"
#include "mapcssstyle_p.h"

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QScopeGuard>

#include <cstring>

template <char Q>
[[nodiscard]] static char* unquoteString(const char *str)
{
    const auto size = strlen(str) - 2;
    if (size <= 0) {
        return nullptr;
    }
    auto out = (char*)malloc(size + 1);
    memset(out, 0, size + 1);
    auto outIt = out;
    for (auto it = str + 1; it < str + size + 1; ++it, ++outIt) {
        if (*it == '\\') {
            ++it;
            switch (*it) {
                case '\\':
                case Q:
                    *outIt = *it; break;
                case 'n':
                    *outIt = '\n'; break;
                case 't':
                    *outIt = '\t'; break;
                default:
                    *outIt++ = '\\';
                    *outIt = *it;
            }
        } else {
            *outIt = *it;
        }
    }
    return out;
}

[[nodiscard]] char* unquoteSingleQuotedString(const char *str)
{
    return unquoteString<'\''>(str);
}

[[nodiscard]] char* unquoteDoubleQuotedString(const char *str)
{
    return unquoteString<'"'>(str);
}

using namespace KOSMIndoorMap;

MapCSSParser::MapCSSParser()
    : d(new MapCSSParserPrivate)
{
}

MapCSSParser::~MapCSSParser() = default;

bool MapCSSParser::hasError() const
{
    return d->m_error;
}

QString MapCSSParser::fileName() const
{
    return d->m_currentFileName;
}

QString MapCSSParser::errorMessage() const
{
    if (!d->m_error) {
        return {};
    }

    return d->m_errorMsg + QLatin1String(": ") + fileName() + QLatin1Char(':') + QString::number(d->m_line) + QLatin1Char(':') + QString::number(d->m_column);
}

MapCSSStyle MapCSSParser::parse(const QString &fileName)
{
    d->m_error = true;

    MapCSSStyle style;
    d->parse(&style, fileName, {});
    if (d->m_error) {
        return MapCSSStyle();
    }

    return style;
}

void MapCSSParserPrivate::parse(MapCSSStyle *style, const QString &fileName, ClassSelectorKey importClass)
{
    QFile f(fileName);
    if (!f.open(QFile::ReadOnly)) {
        qCWarning(Log) << f.fileName() << f.errorString();
        m_error = true;
        m_errorMsg = f.errorString();
        return;
    }
    m_currentFileName = fileName;
    m_currentStyle = style;
    m_importClass = importClass;

    yyscan_t scanner;
    if (yylex_init(&scanner)) {
        return;
    }
    const auto lexerCleanup = qScopeGuard([&scanner]{ yylex_destroy(scanner); });

    const auto b = f.readAll();
    YY_BUFFER_STATE state;
    state = yy_scan_string(b.constData(), scanner);
    if (yyparse(this, scanner)) {
        m_error = true;
        return;
    }

    yy_delete_buffer(state, scanner);

    m_error = false;
    m_currentStyle = nullptr;
    m_importClass = {};
}

bool MapCSSParserPrivate::addImport(char* fileName, ClassSelectorKey importClass)
{
    auto cssFile = QString::fromUtf8(fileName);
    free(fileName);

    if (QFileInfo(cssFile).isRelative()) {
        cssFile = QFileInfo(m_currentFileName).absolutePath() + QLatin1Char('/') + cssFile;
    }

    MapCSSParser p;
    p.d->parse(m_currentStyle, cssFile, importClass);
    if (p.hasError()) {
        m_error = p.d->m_error;
        m_errorMsg = p.errorMessage();
    }
    return !p.hasError();
}

void MapCSSParserPrivate::addRule(MapCSSRule *rule)
{
    if (!m_importClass.isNull()) {
        auto decl = new MapCSSDeclaration(MapCSSDeclaration::ClassDeclaration);
        decl->setClassSelectorKey(m_importClass);
        rule->addDeclaration(decl);
    }
    MapCSSStylePrivate::get(m_currentStyle)->m_rules.push_back(std::unique_ptr<MapCSSRule>(rule));
}

void MapCSSParserPrivate::setError(const QString &msg, int line, int column)
{
    m_error = true;
    m_errorMsg = msg;
    m_line = line;
    m_column = column;
}

ClassSelectorKey MapCSSParserPrivate::makeClassSelector(const char *str, std::size_t len) const
{
    return MapCSSStylePrivate::get(m_currentStyle)->m_classSelectorRegistry.makeKey(str, len, OSM::StringMemory::Transient);
}

LayerSelectorKey MapCSSParserPrivate::makeLayerSelector(const char *str, std::size_t len) const
{
    if (!str || std::strcmp(str, "default") == 0) {
        return {};
    }
    return MapCSSStylePrivate::get(m_currentStyle)->m_layerSelectorRegistry.makeKey(str, len, OSM::StringMemory::Transient);
}
