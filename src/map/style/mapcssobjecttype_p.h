/*
    SPDX-FileCopyrightText: 2020-2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORMAP_MAPCSSOBJECTTYPE_P_H
#define KOSMINDOORMAP_MAPCSSOBJECTTYPE_P_H

namespace KOSMIndoorMap {
enum class MapCSSObjectType {
    Node,
    Way,
    Relation,
    Area,
    Line,
    LineOrArea, // closed way without area tag, meaning depends on context
    Canvas,
    Any
};
}

#endif
