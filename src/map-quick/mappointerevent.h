/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORMAP_MAPPOINTEREVENT_H
#define KOSMINDOORMAP_MAPPOINTEREVENT_H

#include "osmelement.h"

#include <QPointF>
#include <QQmlEngine>

namespace KOSMIndoorMap {

/** Data type for mouse/touch events on the map. */
class MapPointerEvent
{
    Q_GADGET
    QML_ELEMENT
    QML_VALUE_TYPE(mapPointerEvent)
    QML_STRUCTURED_VALUE
    Q_PROPERTY(KOSMIndoorMap::OSMElement element MEMBER m_element)
    Q_PROPERTY(QPointF geoPosition MEMBER m_geoPos)
    Q_PROPERTY(QPointF screenPosition MEMBER m_screenPos)
    Q_PROPERTY(Qt::MouseButton button MEMBER m_button)
    Q_PROPERTY(int modifiers MEMBER m_modifiers)

private:
    OSMElement m_element;
    QPointF m_geoPos;
    QPointF m_screenPos;
    Qt::MouseButton m_button = Qt::NoButton;
    int m_modifiers = 0;
};

}

#endif // KOSMINDOORMAP_MAPPOINTEREVENT_H
