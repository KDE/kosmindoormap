/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORMAP_MAPCSSELEMENTSTATE_H
#define KOSMINDOORMAP_MAPCSSELEMENTSTATE_H

#include <QFlags>

namespace KOSMIndoorMap {

/** Element states accessible via pseudo-classes. */
enum class MapCSSElementState {
    NoState = 0,
    Active = 1, /// element is selected
    Hovered = 2, /// element is hovered
};

Q_DECLARE_FLAGS(MapCSSElementStates, MapCSSElementState)

}

Q_DECLARE_OPERATORS_FOR_FLAGS(KOSMIndoorMap::MapCSSElementStates)

#endif
