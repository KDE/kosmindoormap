/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "osmelement.h"

#include <QJSValueIterator>
#include <QLocale>

using namespace KOSMIndoorMap;

OSMElement::OSMElement() = default;
OSMElement::OSMElement(OSM::Element e)
    : m_element(e)
{
}

OSMElement::~OSMElement() = default;

bool OSMElement::isNull() const
{
    return m_element.type() == OSM::Type::Null;
}

qint64 OSMElement::id() const
{
    return m_element.id();
}

QString OSMElement::name() const
{
    return QString::fromUtf8(m_element.tagValue(OSM::Languages::fromQLocale(QLocale()), "name"));
}

QString OSMElement::url() const
{
    return m_element.url();
}

QPointF OSMElement::center() const
{
    const auto c = m_element.center();
    return {c.lonF(), c.latF() };
}

QString OSMElement::tagValue(const QJSValue &key) const
{
    if (key.isString()) {
        return QString::fromUtf8(m_element.tagValue(key.toString().toUtf8().constData()));
    }
    if (key.isArray()) {
        for (QJSValueIterator it(key); it.hasNext();) {
            it.next();
            auto v = m_element.tagValue(it.value().toString().toUtf8().constData());
            if (!v.isEmpty()) {
                return QString::fromUtf8(v);
            }
        }
    }

    return {};
}

OSM::Element OSMElement::element() const
{
    return m_element;
}

#include "moc_osmelement.cpp"
