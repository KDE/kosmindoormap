/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mapcssexpression_p.h"
#include "mapcssparser_impl.h"
#include "mapcssparsercontext_p.h"
#include "mapcssscanner.h"
#include "mapcssterm_p.h"
#include "logging.h"

namespace KOSMIndoorMap {
class MapCSSExpressionParserContext : public MapCSSParserContext {
public:
    MapCSSExpressionParserContext() : MapCSSParserContext(ParseEvalExpression) {};
};
}

using namespace KOSMIndoorMap;

MapCSSExpression::MapCSSExpression() = default;
MapCSSExpression::MapCSSExpression(MapCSSExpression&&) noexcept = default;

MapCSSExpression::MapCSSExpression(MapCSSTerm *term)
    : m_term(term)
{
}

MapCSSExpression::~MapCSSExpression() = default;

MapCSSExpression& MapCSSExpression::operator=(MapCSSExpression&&) noexcept = default;

bool MapCSSExpression::isValid() const
{
    return (bool)m_term;
}

void MapCSSExpression::compile(const OSM::DataSet &dataSet)
{
    m_term->compile(dataSet);
}

MapCSSValue MapCSSExpression::evaluate(const MapCSSState &state, const MapCSSResultLayer &result) const
{
    return m_term->evaluate(state, result);
}

void MapCSSExpression::write(QIODevice *out) const
{
    m_term->write(out);
}

MapCSSExpression MapCSSExpression::fromString(const char *str)
{
    MapCSSExpressionParserContext context;
    yyscan_t scanner;

    if (yylex_init_extra(&context, &scanner)) {
        return {};
    }
    const auto lexerCleanup = qScopeGuard([&scanner]{ yylex_destroy(scanner); });

    YY_BUFFER_STATE state;
    state = yy_scan_string(str, scanner);
    if (yyparse(&context, scanner)) {
        // TODO store error information from context in result
        return {};
    }

    yy_delete_buffer(state, scanner);

    return MapCSSExpression(context.m_term);
}
