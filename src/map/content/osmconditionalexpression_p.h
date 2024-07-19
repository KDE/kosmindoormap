/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORMAP_OSM_CONDITIONAL_EXPRESSION_H
#define KOSMINDOORMAP_OSM_CONDITIONAL_EXPRESSION_H

#include <QByteArray>

#include <vector>

namespace KOSMIndoorMap {

class OSMConditionalExpressionContext;

/** A conditional tag expression.
 *  @see https://wiki.openstreetmap.org/wiki/Conditional_restrictions.
 */
class OSMConditionalExpression
{
public:
    explicit OSMConditionalExpression();
    OSMConditionalExpression(const OSMConditionalExpression&) = delete;
    ~OSMConditionalExpression();
    OSMConditionalExpression& operator=(const OSMConditionalExpression&) = delete;

    // TODO error handling
    void parse(const QByteArray &expression);

    [[nodiscard]] QByteArray evaluate(const OSMConditionalExpressionContext &context) const;

private:
    struct Condition {
        QByteArray value;
        QByteArray condition;
    };
    std::vector<Condition> m_conditions;
};

}

#endif
