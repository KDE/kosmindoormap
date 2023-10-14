/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSM_EDITORCONTROLLER_H
#define KOSM_EDITORCONTROLLER_H

#include <osm/element.h>

#include <memory>

namespace KOSM {

/** Filtering/sorting on top of the AmenityModel.
 *  - filters on all visible roles
 *  - sorts while keeping the grouping intact
 */
class EditorController
{
    Q_GADGET
public:
    enum Editor {
        ID,
        JOSM,
        Vespucci
    };
    Q_ENUM(Editor)

    Q_INVOKABLE static bool hasEditor(Editor editor);
    Q_INVOKABLE static void editElement(OSM::Element element, Editor editor);
};

}

#endif
