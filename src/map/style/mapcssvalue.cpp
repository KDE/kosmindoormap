/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mapcssvalue_p.h"

#include <QIODevice>

#include <cmath>

using namespace KOSMIndoorMap;

MapCSSValue::MapCSSValue() = default;

MapCSSValue::MapCSSValue(const QByteArray &str)
    : m_value(str)
{
}

MapCSSValue::MapCSSValue(double num)
    : m_value(num)
{}

MapCSSValue::MapCSSValue(bool b)
    : m_value(b)
{}

MapCSSValue::~MapCSSValue() = default;

bool MapCSSValue::isNone() const
{
    switch (m_value.typeId()) {
        case QMetaType::QByteArray:
            return m_value.toByteArray().isEmpty();
        case QMetaType::Double:
            return m_value.toDouble() == 0.0 || std::isnan(m_value.toDouble());
    }
    return m_value.isNull();
}

QByteArray MapCSSValue::asString() const
{
    switch (m_value.typeId()) {
        case QMetaType::QByteArray:
            return m_value.toByteArray();
        case QMetaType::Double:
            return QByteArray::number(m_value.toDouble());
        case QMetaType::Bool:
            return m_value.toBool() ? "true" : "false";
    }
    return {};
}

double MapCSSValue::asNumber() const
{
    switch (m_value.typeId()) {
        case QMetaType::QByteArray: {
            const auto b = m_value.toByteArray();
            if (b.isEmpty()) {
                return 0.0;
            }
            bool ok = false;
            auto n = b.toDouble(&ok);
            return ok ? n : NAN;
        }
        case QMetaType::Double:
            return m_value.toDouble();
    }

    return NAN;
}

bool MapCSSValue::asBoolean() const
{
    switch (m_value.typeId()) {
        case QMetaType::QByteArray:
        {
            const auto b = m_value.toByteArray();
            return b != "false" && b != "0" && b != "no" && !b.isEmpty();
        }
        case QMetaType::Double:
            return m_value.toDouble() != 0.0;
        case QMetaType::Bool:
            return m_value.toBool();
    }

    return false;
}

bool MapCSSValue::compareEqual(const MapCSSValue &other) const
{
    if (m_value.typeId() != other.m_value.typeId()) {
        return asString() == other.asString();
    }

    switch (m_value.typeId()) {
        case QMetaType::QByteArray:
            return m_value.toByteArray() == other.m_value.toByteArray();
        case QMetaType::Double:
            return m_value.toDouble() == other.m_value.toDouble();
        case QMetaType::Bool:
            return m_value.toBool() == other.m_value.toBool();
    }

    return false;
}

void MapCSSValue::write(QIODevice *out) const
{
    switch (m_value.typeId()) {
        case QMetaType::QByteArray:
            out->write(m_value.toByteArray());
            break;
        case QMetaType::Double:
            out->write(QByteArray::number(m_value.toDouble()));
            break;
    }
}
