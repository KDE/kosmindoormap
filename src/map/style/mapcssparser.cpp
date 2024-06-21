/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mapcssparser.h"
#include "mapcssparser_p.h"
#include "logging.h"

#include "mapcssloader.h"
#include "mapcssparser_impl.h"
#include "mapcssdeclaration_p.h"
#include "mapcssscanner.h"
#include "mapcssstyle.h"

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
    return d->m_error != MapCSSParser::NoError;
}

MapCSSParser::Error MapCSSParser::error() const
{
    return d->m_error;
}

QUrl MapCSSParser::url() const
{
    return d->m_currentUrl;
}

QString MapCSSParser::errorMessage() const
{
    if (!d->m_error) {
        return {};
    }

    return d->m_errorMsg + QLatin1String(": ") + url().toString() + QLatin1Char(':') + QString::number(d->m_line) + QLatin1Char(':') + QString::number(d->m_column);
}

MapCSSStyle MapCSSParser::parse(const QString &fileName)
{
    return parse(MapCSSLoader::resolve(fileName));
}

MapCSSStyle MapCSSParser::parse(const QUrl &url)
{
    MapCSSStyle style;
    d->parse(&style, url, {});
    if (d->m_error) {
        return MapCSSStyle();
    }

    return style;
}

void MapCSSParserPrivate::parse(MapCSSStyle *style, const QUrl &url, ClassSelectorKey importClass)
{
    const auto fileName = MapCSSLoader::toLocalFile(url);
    QFile f(fileName);
    if (!f.open(QFile::ReadOnly)) {
        qCWarning(Log) << f.fileName() << f.errorString();
        if (!QFile::exists(fileName)) {
            m_error = MapCSSParser::FileNotFoundError;
            m_currentUrl = url;
        } else {
            m_error = MapCSSParser::FileIOError;
        }
        m_errorMsg = f.errorString();
        return;
    }
    m_currentUrl = url;
    m_currentStyle = style;
    m_importClass = importClass;

    yyscan_t scanner;
    if (yylex_init_extra(this, &scanner)) {
        return;
    }
    const auto lexerCleanup = qScopeGuard([&scanner]{ yylex_destroy(scanner); });

    const auto b = f.readAll();
    YY_BUFFER_STATE state;
    state = yy_scan_string(b.constData(), scanner);
    m_error = MapCSSParser::SyntaxError;
    if (yyparse(this, scanner)) {
        return;
    }

    yy_delete_buffer(state, scanner);

    m_error = MapCSSParser::NoError;
    m_currentStyle = nullptr;
    m_importClass = {};
}
