/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORMAP_MAPCSSEXPRESSIONCONTEXT_P_H
#define KOSMINDOORMAP_MAPCSSEXPRESSIONCONTEXT_P_H

#include "kosmindoormap_export.h"

namespace KOSMIndoorMap {

class MapCSSResultLayer;
class MapCSSState;

/** Context in which a MapCSSExpression is evaluated. */
class MapCSSExpressionContext {
public:
    const MapCSSState &state;
    const MapCSSResultLayer &result;
};

}

#endif
