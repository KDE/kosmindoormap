/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORMAP_SCENEGRAPHDUMPER_H
#define KOSMINDOORMAP_SCENEGRAPHDUMPER_H

#include "kosmindoormap_export.h"

#include <KOSMIndoorMap/SceneGraphItem>

class QIODevice;

namespace KOSMIndoorMap {

class SceneGraph;
class View;

/** "Renderer" that writes the scene graph to a file for debugging.
 *  This could potentially share its top-level logic with PainterRenderer
 */
class KOSMINDOORMAP_EXPORT SceneGraphDumper
{
public:
    explicit SceneGraphDumper();
    ~SceneGraphDumper();

    void render(const SceneGraph &sg, View *view);

private:
    void beginPhase(SceneGraphItemPayload::RenderPhase phase);

    View *m_view = nullptr;

    std::vector<const SceneGraphItem*> m_renderBatch; // member rather than function-local to preserve allocations
    QIODevice *m_io;
};

}

#endif // KOSMINDOORMAP_SCENEGRAPHDUMPER_H
