/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mapcssterm_p.h"

#include "mapcssdeclaration_p.h"
#include "mapcssexpressioncontext_p.h"
#include "mapcssresult.h"
#include "mapcssstate_p.h"
#include "logging.h"

#include "content/osmconditionalexpression_p.h"
#include "content/osmconditionalexpressioncontext_p.h"

#include <QIODevice>

#include <cstdint>
#include <limits>
#include <span>

using namespace KOSMIndoorMap;

MapCSSTerm::MapCSSTerm(Operation op)
    : m_op(op)
{
}

MapCSSTerm::~MapCSSTerm() = default;

void MapCSSTerm::addChildTerm(MapCSSTerm *term)
{
    m_children.emplace_back(term);
}

// ### alphabetically sorted
struct {
    const char *name;
    MapCSSTerm::Operation op;
} static constexpr const function_name_map[] = {
    { "KOSM_conditional", MapCSSTerm::KOSM_Conditional },
    { "any", MapCSSTerm::Any },
    { "boolean", MapCSSTerm::BooleanCast },
    { "concat", MapCSSTerm::Concatenate },
    { "cond", MapCSSTerm::Conditional },
    { "int", MapCSSTerm::Integer },
    { "max", MapCSSTerm::Maximum },
    { "metric", MapCSSTerm::Metric },
    { "min", MapCSSTerm::Minimum },
    { "num", MapCSSTerm::NumericalCast },
    { "prop", MapCSSTerm::ReadProperty },
    { "replace", MapCSSTerm::Replace },
    { "sqrt", MapCSSTerm::Sqrt },
    { "str", MapCSSTerm::StringCast },
    { "tag", MapCSSTerm::ReadTag },
    { "zmetric", MapCSSTerm::ZMetric },
};

MapCSSTerm::Operation MapCSSTerm::parseOperation(const char *str, std::size_t len)
{
    const auto it = std::lower_bound(std::begin(function_name_map), std::end(function_name_map), std::span(str, len), [](const auto &m, const auto &name) {
        return std::strncmp(m.name, name.data(), name.size()) < 0;
    });
    if (it != std::end(function_name_map) && std::strncmp((*it).name, str, len) == 0) {
        return (*it).op;
    }
    return Unknown;
}

// ### sorted by operation
struct {
    uint16_t minArgs;
    uint16_t maxArgs;
} static constexpr const argument_count_map[] = {
    { 0, 0 },
    { 0, 0 },
    // numerical operations
    { 2, 2 },
    { 2, 2 },
    { 2, 2 },
    { 2, 2 },
    // logical operations
    { 2, 2 },
    { 2, 2 },
    { 1, 1 },
    // comparisions
    { 2, 2 },
    { 2, 2 },
    { 2, 2 },
    { 2, 2 },
    { 2, 2 },
    { 2, 2 },
    // casts
    { 1, 1 },
    { 1, 1 },
    { 1, 1 },
    // conditionals
    { 3, 3 },
    { 1, std::numeric_limits<uint16_t>::max() },
    // string functions
    { 2, std::numeric_limits<uint16_t>::max() },
    { 3, 3 },
    // numerical functions
    { 1, 1 },
    { 2, std::numeric_limits<uint16_t>::max() },
    { 2, std::numeric_limits<uint16_t>::max() },
    { 1, 1 },
    // unit conversions
    { 1, 1 },
    { 1, 1 },
    // list functions
    // data/style state access functions
    { 1, 1 },
    { 1, 1 },
    // our own extensions
    { 1, 1 },
};

bool MapCSSTerm::validChildCount() const
{
    return m_children.size() >= argument_count_map[m_op].minArgs && m_children.size() <= argument_count_map[m_op].maxArgs;
}

void MapCSSTerm::compile(const OSM::DataSet &dataSet)
{
    for (const auto &c : m_children) {
        c->compile(dataSet);
    }

    // TODO resolve tag key in case of m_op == ReadTag and m_children[0] being a constant expression
    // TODO resolve property name in case of m_op == ReadProperty and m_children[0] being a constant expression
}

MapCSSValue MapCSSTerm::evaluate(const MapCSSExpressionContext &context) const
{
    switch (m_op) {
        case Unknown:
            return {};
        case Literal:
            return m_literal;

        case Addition:
            return m_children[0]->evaluate(context).asNumber() + m_children[1]->evaluate(context).asNumber();
        case Subtraction:
            return m_children[0]->evaluate(context).asNumber() - m_children[1]->evaluate(context).asNumber();
        case Multiplication:
            return m_children[0]->evaluate(context).asNumber() * m_children[1]->evaluate(context).asNumber();
        case Division: {
            const auto divisor = m_children[1]->evaluate(context);
            if (!divisor.isNone() && divisor.asNumber() != 0.0) {
                return m_children[0]->evaluate(context).asNumber() / divisor.asNumber();
            }
            return {};
        }

        case LogicalAnd:
            return m_children[0]->evaluate(context).asBoolean() && m_children[1]->evaluate(context).asBoolean();
        case LogicalOr:
            return m_children[0]->evaluate(context).asBoolean() || m_children[1]->evaluate(context).asBoolean();
        case LogicalNot:
            return !m_children[0]->evaluate(context).asBoolean();

        case CompareEqual:
            return m_children[0]->evaluate(context).compareEqual(m_children[1]->evaluate(context));
        case CompareNotEqual:
            return !m_children[0]->evaluate(context).compareEqual(m_children[1]->evaluate(context));
        case CompareLess:
            return m_children[0]->evaluate(context).asNumber() < m_children[1]->evaluate(context).asNumber();
        case CompareGreater:
            return m_children[0]->evaluate(context).asNumber() > m_children[1]->evaluate(context).asNumber();
        case CompareLessOrEqual:
            return m_children[0]->evaluate(context).asNumber() <= m_children[1]->evaluate(context).asNumber();
        case CompareGreaterOrEqual:
            return m_children[0]->evaluate(context).asNumber() >= m_children[1]->evaluate(context).asNumber();

        case NumericalCast:
            return m_children[0]->evaluate(context).asNumber();
        case StringCast:
            return m_children[0]->evaluate(context).asString();
        case BooleanCast:
            return m_children[0]->evaluate(context).asBoolean();

        case Conditional:
            if (m_children[0]->evaluate(context).asBoolean()) {
                return m_children[1]->evaluate(context);
            } else {
                return m_children[2]->evaluate(context);
            }
            break;
        case Any: {
            for (const auto &child : m_children) {
                auto r = child->evaluate(context);
                if (!r.isNone()) {
                    return r;
                }
            }
            return {};
        }

        case Concatenate:
        {
            QByteArray s;
            for (const auto &child : m_children) {
                s += child->evaluate(context).asString();
            }
            return s;
        }
        case Replace:
            return m_children[0]->evaluate(context).asString().replace(m_children[1]->evaluate(context).asString(), m_children[2]->evaluate(context).asString());

        case Integer: {
            const auto i = (int)m_children[0]->evaluate(context).asNumber();
            return (double)i;
        }
        case Maximum: {
            double r = std::numeric_limits<double>::lowest();
            for (const auto &child : m_children) {
                r = std::max(r, child->evaluate(context).asNumber());
            }
            return r;
        }
        case Minimum: {
            double r = std::numeric_limits<double>::max();
            for (const auto &child : m_children) {
                r = std::min(r, child->evaluate(context).asNumber());
            }
            return r;
        }
        case Sqrt:
            return std::sqrt(m_children[0]->evaluate(context).asNumber());

        case Metric:
        case ZMetric:
            return m_children[0]->evaluate(context); // our unit handling should deal with this already

        case ReadProperty: {
            const auto propName = m_children[0]->evaluate(context).asString();
            const auto prop = MapCSSDeclaration::propertyFromName(propName.constData(), propName.size());
            const auto decl = context.result.declaration(prop);
            if (!decl || !decl->isValid()) {
                return {};
            }
            if (decl->hasExpression()) {
                qCWarning(Log) << "Recursive eval() expression not supported, evaluation aborted";
                return {};
            }
            return decl->stringValue().toUtf8(); // TODO support other property types
        }
        case ReadTag:
        {
            const auto v = context.result.resolvedTagValue(m_children[0]->evaluate(context).asString().constData(), context.state);
            return v ? *v : MapCSSValue();
        }
        case KOSM_Conditional:
        {
            OSMConditionalExpression expr; // TODO cache those
            expr.parse(m_children[0]->evaluate(context).asString());

            OSMConditionalExpressionContext condContext;
            condContext.element = context.state.element;
            condContext.openingHoursCache = context.state.openingHours;
            return expr.evaluate(condContext);
        }
    }

    return {};
}

struct {
    MapCSSTerm::Operation op;
    const char *name;
} static constexpr const infix_output_map[] = {
    { MapCSSTerm::Addition, "+" },
    { MapCSSTerm::Subtraction, "-" },
    { MapCSSTerm::Multiplication, "*" },
    { MapCSSTerm::Division, "/" },
    { MapCSSTerm::LogicalAnd, "&&" },
    { MapCSSTerm::LogicalOr, "||" },
    { MapCSSTerm::CompareEqual, "==" },
    { MapCSSTerm::CompareNotEqual, "!=" },
    { MapCSSTerm::CompareLess, "<" },
    { MapCSSTerm::CompareGreater, ">" },
    { MapCSSTerm::CompareLessOrEqual, "<=", },
    { MapCSSTerm::CompareGreaterOrEqual, ">=" },
};

void MapCSSTerm::write(QIODevice *out) const
{
    if (m_op == Unknown) {
        return;
    }
    if (m_op == Literal) {
        m_literal.write(out);
        return;
    }
    if (m_op == LogicalNot) {
        out->write("!");
        m_children[0]->write(out);
    }

    for (const auto &m : infix_output_map) {
        if (m.op == m_op) {
            out->write("(");
            m_children[0]->write(out);
            out->write(m.name);
            m_children[1]->write(out);
            out->write(")");
            return;
        }
    }

    for (const auto &m : function_name_map) {
        if (m.op == m_op) {
            out->write(m.name);
            out->write("(");
            if (!m_children.empty()) {
                for (auto it = m_children.begin(); it != std::prev(m_children.end()); ++it) {
                    (*it)->write(out);
                    out->write(", ");
                }
                m_children.back()->write(out);
            }
            out->write(")");
            return;
        }
    }
}
