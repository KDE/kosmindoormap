/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORMAP_MAPCSSTERM_P_H
#define KOSMINDOORMAP_MAPCSSTERM_P_H

#include "mapcssvalue_p.h"

#include <memory>
#include <vector>

class QIODevice;

namespace OSM { class DataSet; }

namespace KOSMIndoorMap {

class MapCSSExpressionContext;

/** Part of a MapCSS eval() expression. */
class MapCSSTerm {
public:
    enum Operation {
        Unknown,
        Literal,

        // numerical operations
        Addition,
        Subtraction,
        Multiplication,
        Division,

        // logical operations
        LogicalAnd,
        LogicalOr,
        LogicalNot,

        // comparision
        CompareEqual,
        CompareNotEqual,
        CompareLess,
        CompareGreater,
        CompareLessOrEqual,
        CompareGreaterOrEqual,

        // type casts
        NumericalCast,
        StringCast,
        BooleanCast,

        // conditionals
        Conditional,
        Any,

        // string functions
        Concatenate,
        Replace, // JOSM extension

        // numerical functions
        Integer,
        Maximum,
        Minimum,
        Sqrt,

        // unit conversions
        Metric,
        ZMetric,

        // list functions (JOSM extensions)
        // TODO

        // functions accessing element or style state
        ReadProperty,
        ReadTag,
    };

    explicit MapCSSTerm(Operation op = Operation::Unknown);
    explicit inline MapCSSTerm(Operation op, std::initializer_list<MapCSSTerm*> children)
        : MapCSSTerm(op)
    {
        m_children.reserve(children.size());
        std::for_each(children.begin(), children.end(), [this](auto *c) { addChildTerm(c); });
    }
    ~MapCSSTerm();

    [[nodiscard]] static Operation parseOperation(const char *str, std::size_t len);

    /** Check whether the number of sub-terms matches this oepration. */
    [[nodiscard]] bool validChildCount() const;

    void addChildTerm(MapCSSTerm *term);

    /** Resolve tag keys etc. */
    void compile(const OSM::DataSet &dataSet);

    /** Evaluate this sub-expression under the given context. */
    [[nodiscard]] MapCSSValue evaluate(const MapCSSExpressionContext &context) const;

    void write(QIODevice *out) const;

    Operation m_op = Unknown;
    std::vector<std::unique_ptr<MapCSSTerm>> m_children;
    MapCSSValue m_literal;
};

}

#endif
