/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORMAP_OSMELEMENT_H
#define KOSMINDOORMAP_OSMELEMENT_H

#include <osm/element.h>

#include <QJSValue>
#include <QMetaType>
#include <QPointF>
#include <QUrl>

namespace KOSMIndoorMap {

/** QML wrapper around an OSM element. */
class OSMElement
{
    Q_GADGET
    Q_PROPERTY(bool isNull READ isNull)
    Q_PROPERTY(QString name READ name)
    Q_PROPERTY(QString url READ url)
    Q_PROPERTY(OSM::Element element READ element)
    Q_PROPERTY(QPointF center READ center)
public:
    OSMElement();
    explicit OSMElement(OSM::Element e);
    ~OSMElement();

    [[nodiscard]] bool isNull() const;
    [[nodiscard]] QString name() const;
    [[nodiscard]] QString url() const;
    [[nodiscard]] QPointF center() const;

    [[nodiscard]] Q_INVOKABLE QString tagValue(const QJSValue &key) const;

    // @internal
    [[nodiscard]] OSM::Element element() const;

private:
    OSM::Element m_element;
};

}

Q_DECLARE_METATYPE(KOSMIndoorMap::OSMElement)

#endif // KOSMINDOORMAP_OSMELEMENT_H
