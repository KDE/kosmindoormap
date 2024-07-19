/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "osmconditionalexpression_p.h"
#include "osmconditionalexpressioncontext_p.h"

#include <scene/openinghourscache_p.h>

using namespace KOSMIndoorMap;

OSMConditionalExpression::OSMConditionalExpression() = default;
OSMConditionalExpression::~OSMConditionalExpression() = default;

void OSMConditionalExpression::parse(const QByteArray &expression)
{
    QByteArrayView input(expression);
    while (!input.isEmpty()) {
        const auto sep = input.lastIndexOf('@');
        if (sep < 0) {
            break;
        }
        const auto start = input.lastIndexOf(';', sep);
        m_conditions.push_back(Condition{
            .value = input.mid(start + 1, sep - 1 - start).trimmed().toByteArray(),
            .condition = input.mid(sep + 1).trimmed().toByteArray()
        });
        input = input.left(std::max<qsizetype>(start, 0));
    }

    // we parse back to front, but evaluation order matters
    std::reverse(m_conditions.begin(), m_conditions.end());
}

QByteArray OSMConditionalExpression::evaluate(const OSMConditionalExpressionContext &context) const
{
    for (const auto &cond : m_conditions) {
        if (context.openingHoursCache->isAtCurrentTime(context.element, cond.condition)) {
            return cond.value;
        }
    }
    return {};
}
