/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORROUTING_NAVMESHBUILDER_H
#define KOSMINDOORROUTING_NAVMESHBUILDER_H

#include "kosmindoorrouting_export.h"

#include <QObject>

#include <memory>

namespace KOSMIndoorMap {
class AbstractOverlaySource;
class MapData;
}

namespace KOSMIndoorRouting {

class NavMesh;
class NavMeshBuilderPrivate;

/** Job for building a navigation mesh for the given building. */
class KOSMINDOORROUTING_EXPORT NavMeshBuilder : public QObject
{
    Q_OBJECT
public:
    explicit NavMeshBuilder(QObject *parent = nullptr);
    ~NavMeshBuilder();

    void setMapData(const KOSMIndoorMap::MapData &mapData);
    void setEquipmentModel(KOSMIndoorMap::AbstractOverlaySource *equipmentModel);

    void writeDebugNavMesh(const QString &gsetFile, const QString &objFile);

    void start();

    [[nodiscard]] NavMesh navMesh() const;

Q_SIGNALS:
    void finished();

private:
    std::unique_ptr<NavMeshBuilderPrivate> d;
};
}

#endif
