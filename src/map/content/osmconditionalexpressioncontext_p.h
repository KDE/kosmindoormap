/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORMAP_OSM_CONDITIONAL_EXPRESSION_CONTEXT_H
#define KOSMINDOORMAP_OSM_CONDITIONAL_EXPRESSION_CONTEXT_H

#include <KOSM/Element>

namespace KOSMIndoorMap {

class OpeningHoursCache;

class OSMConditionalExpressionContext
{
public:
    OSM::Element element;
    OpeningHoursCache *openingHoursCache = nullptr;
};

}

#endif
