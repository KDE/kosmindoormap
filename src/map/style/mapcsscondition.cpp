/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mapcsscondition_p.h"
#include "mapcssstate_p.h"

#include <QDebug>
#include <QIODevice>

using namespace KOSMIndoorMap;

MapCSSCondition::MapCSSCondition() = default;
MapCSSCondition::MapCSSCondition(MapCSSCondition &&) = default;
MapCSSCondition::~MapCSSCondition() = default;

static double toNumber(const QByteArray &val)
{
    bool res = false;
    const auto n = val.toDouble(&res);
    return res ? n : NAN;
}

void MapCSSCondition::compile(const OSM::DataSet &dataSet)
{
    if (m_key == "mx:closed") {
        m_tagKey = dataSet.tagKey("opening_hours");
        m_op = (m_op == KeyNotSet ? IsNotClosed : IsClosed);
    } else {
        m_tagKey = dataSet.tagKey(m_key.constData());
    }

    switch(m_op) {
        case KeySet:
        case KeyNotSet:
            break;
        case Equal:
        case NotEqual:
            if (m_value.isEmpty() && std::isnan(m_numericValue)) {
                qWarning() << "Empty comparison, use key (not) set operation instead!";
            }
            break;
        case LessThan:
        case GreaterThan:
        case LessOrEqual:
        case GreaterOrEqual:
            if (std::isnan(m_numericValue)) {
                qWarning() << "Numeric comparison without numeric value set!";
            }
            break;
        case IsClosed:
        case IsNotClosed:
            break;
    }
}

bool MapCSSCondition::matches(const MapCSSState &state) const
{
    if (m_tagKey.isNull()) {
        // if we couldn't compile the tag, it doesn't exist and thus can never be set
        return m_op == KeyNotSet || m_op == NotEqual;
    }

    // this method is such a hot path that even the ref/deref in QByteArray for OSM::Element::tagValue matters
    // so we do tag lookup manually here
    const auto tagEnd = state.element.tagsEnd();
    const auto tagIt = std::lower_bound(state.element.tagsBegin(), tagEnd, m_tagKey);
    const auto tagIsSet = (tagIt != tagEnd && (*tagIt).key == m_tagKey);
    switch (m_op) {
        case KeySet:
            return tagIsSet;
        case KeyNotSet:
            return !tagIsSet;
        case Equal:
            if (std::isnan(m_numericValue)) {
                return !tagIsSet ? false : (*tagIt).value == m_value;
            }
            return !tagIsSet ? false : toNumber((*tagIt).value) == m_numericValue;
        case NotEqual:
            if (std::isnan(m_numericValue)) {
                return !tagIsSet ? true : (*tagIt).value != m_value;
            }
            return !tagIsSet ? true : toNumber((*tagIt).value) != m_numericValue;
        case LessThan: return !tagIsSet ? false : toNumber((*tagIt).value) < m_numericValue;
        case GreaterThan: return !tagIsSet ? false : toNumber((*tagIt).value) > m_numericValue;
        case LessOrEqual: return !tagIsSet ? false : toNumber((*tagIt).value) <= m_numericValue;
        case GreaterOrEqual: return !tagIsSet ? false : toNumber((*tagIt).value) >= m_numericValue;
        case IsClosed:
        case IsNotClosed:
        {
            if (!tagIsSet || (*tagIt).value.isEmpty() || !state.openingHours) {
                return m_op == IsNotClosed;
            }
            const auto closed = state.openingHours->isClosed(state.element, (*tagIt).value);
            return m_op == IsClosed ? closed : !closed;
        }
    }
    return false;
}

bool MapCSSCondition::matchesCanvas(const MapCSSState &state) const
{
    if (m_key != "level") {
        return false;
    }

    switch (m_op) {
        case KeySet:
        case KeyNotSet:
        case IsClosed:
        case IsNotClosed:
            return false;
        case Equal: return (state.floorLevel/10) == m_numericValue;
        case NotEqual: return (state.floorLevel/10) != m_numericValue;
        case LessThan: return (state.floorLevel/10) < m_numericValue;
        case GreaterThan: return (state.floorLevel/10) > m_numericValue;
        case LessOrEqual: return (state.floorLevel/10) <= m_numericValue;
        case GreaterOrEqual: return (state.floorLevel/10) >= m_numericValue;
    }

    return false;
}

void MapCSSCondition::setKey(const char *key, int len)
{
    m_key = QByteArray(key, len);
}

void MapCSSCondition::setOperation(MapCSSCondition::Operator op)
{
    m_op = op;
}

void MapCSSCondition::setValue(const char *value, int len)
{
    m_value = QByteArray(value, len);
}

void MapCSSCondition::setValue(double val)
{
    m_numericValue = val;
}

void MapCSSCondition::write(QIODevice *out) const
{
    out->write("[");
    if (m_op == KeyNotSet) { out->write("!"); }
    out->write(m_key);

    switch (m_op) {
        case KeySet:
        case KeyNotSet:
        case IsClosed:
        case IsNotClosed:
            out->write("]"); return;
        case Equal: out->write("="); break;
        case NotEqual: out->write("!="); break;
        case LessThan: out->write("<"); break;
        case GreaterThan: out->write(">"); break;
        case LessOrEqual: out->write("<="); break;
        case GreaterOrEqual: out->write(">="); break;
    }

    if (m_numericValue != NAN && m_value.isEmpty()) {
        out->write(QByteArray::number(m_numericValue));
    } else {
        // TODO quote if m_value contains non-identifier chars
        out->write(m_value);
    }

    out->write("]");
}


void MapCSSConditionHolder::addCondition(MapCSSCondition *condition)
{
    conditions.push_back(std::unique_ptr<MapCSSCondition>(condition));
}
