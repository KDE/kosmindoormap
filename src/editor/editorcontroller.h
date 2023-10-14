/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSM_EDITORCONTROLLER_H
#define KOSM_EDITORCONTROLLER_H

#include <osm/element.h>

namespace KOSM {

/** Filtering/sorting on top of the AmenityModel.
 *  - filters on all visible roles
 *  - sorts while keeping the grouping intact
 */
class EditorController
{
    Q_GADGET
public:
    Q_INVOKABLE static void editElement(OSM::Element element);
};

}

#endif
