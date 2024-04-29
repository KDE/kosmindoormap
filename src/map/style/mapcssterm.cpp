/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mapcssterm_p.h"

#include "mapcssdeclaration_p.h"
#include "mapcssresult.h"
#include "mapcssstate_p.h"
#include "logging.h"

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

MapCSSValue MapCSSTerm::evaluate(const MapCSSState &state, const MapCSSResultLayer &result) const
{
    switch (m_op) {
        case Unknown:
            return {};
        case Literal:
            return m_literal;

        case Addition:
            return m_children[0]->evaluate(state, result).asNumber() + m_children[1]->evaluate(state, result).asNumber();
        case Subtraction:
            return m_children[0]->evaluate(state, result).asNumber() - m_children[1]->evaluate(state, result).asNumber();
        case Multiplication:
            return m_children[0]->evaluate(state, result).asNumber() * m_children[1]->evaluate(state, result).asNumber();
        case Division: {
            const auto divisor = m_children[1]->evaluate(state, result);
            if (!divisor.isNone() && divisor.asNumber() != 0.0) {
                return m_children[0]->evaluate(state, result).asNumber() / divisor.asNumber();
            }
            return {};
        }

        case LogicalAnd:
            return m_children[0]->evaluate(state, result).asBoolean() && m_children[1]->evaluate(state, result).asBoolean();
        case LogicalOr:
            return m_children[0]->evaluate(state, result).asBoolean() || m_children[1]->evaluate(state, result).asBoolean();
        case LogicalNot:
            return !m_children[0]->evaluate(state, result).asBoolean();

        case CompareEqual:
            return m_children[0]->evaluate(state, result).compareEqual(m_children[1]->evaluate(state, result));
        case CompareNotEqual:
            return !m_children[0]->evaluate(state, result).compareEqual(m_children[1]->evaluate(state, result));
        case CompareLess:
            return m_children[0]->evaluate(state, result).asNumber() < m_children[1]->evaluate(state, result).asNumber();
        case CompareGreater:
            return m_children[0]->evaluate(state, result).asNumber() > m_children[1]->evaluate(state, result).asNumber();
        case CompareLessOrEqual:
            return m_children[0]->evaluate(state, result).asNumber() <= m_children[1]->evaluate(state, result).asNumber();
        case CompareGreaterOrEqual:
            return m_children[0]->evaluate(state, result).asNumber() >= m_children[1]->evaluate(state, result).asNumber();

        case NumericalCast:
            return m_children[0]->evaluate(state, result).asNumber();
        case StringCast:
            return m_children[0]->evaluate(state, result).asString();
        case BooleanCast:
            return m_children[0]->evaluate(state, result).asBoolean();

        case Conditional:
            if (m_children[0]->evaluate(state, result).asBoolean()) {
                return m_children[1]->evaluate(state, result);
            } else {
                return m_children[2]->evaluate(state, result);
            }
            break;
        case Any: {
            for (const auto &child : m_children) {
                auto r = child->evaluate(state, result);
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
                s += child->evaluate(state, result).asString();
            }
            return s;
        }

        case Integer: {
            const auto i = (int)m_children[0]->evaluate(state, result).asNumber();
            return (double)i;
        }
        case Maximum: {
            double r = std::numeric_limits<double>::lowest();
            for (const auto &child : m_children) {
                r = std::max(r, child->evaluate(state, result).asNumber());
            }
            return r;
        }
        case Minimum: {
            double r = std::numeric_limits<double>::max();
            for (const auto &child : m_children) {
                r = std::min(r, child->evaluate(state, result).asNumber());
            }
            return r;
        }
        case Sqrt:
            return std::sqrt(m_children[0]->evaluate(state, result).asNumber());

        case Metric:
        case ZMetric:
            return m_children[0]->evaluate(state, result); // our unit handling should deal with this already

        case ReadProperty: {
            const auto propName = m_children[0]->evaluate(state, result).asString();
            const auto prop = MapCSSDeclaration::propertyFromName(propName.constData(), propName.size());
            const auto decl = result.declaration(prop);
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
            return state.element.tagValue(m_children[0]->evaluate(state, result).asString().constData());
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
