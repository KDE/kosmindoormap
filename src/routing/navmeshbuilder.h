/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORROUTING_NAVMESHBUILDER_H
#define KOSMINDOORROUTING_NAVMESHBUILDER_H

#include "kosmindoorrouting_export.h"
#include "navmeshtransform.h"

#include <KOSMIndoorMap/MapData>
#include <KOSMIndoorMap/MapCSSResult>
#include <KOSMIndoorMap/MapCSSStyle>

#include <QBuffer>
#include <QByteArray>
#include <QObject>

#include <unordered_map>

namespace KOSMIndoorMap {
class AbstractOverlaySource;
class MapCSSResultLayer;
}

namespace KOSMIndoorRouting {

enum class AreaType {
    Walkable,
    Stairs,
    Elevator,
    Escalator,
};

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

private:
    enum class LinkDirection { Forward, Backward, Bidirectional };

    /** Look up level for a given node id. */
    [[nodiscard]] int levelForNode(OSM::Id nodeId) const;
    void addNodeToLevelIndex(OSM::Id nodeId, int level);
    void indexNodeLevels();

    void processElement(OSM::Element elem, int floorLevel);
    void processGeometry(OSM::Element elem, int floorLevel, const KOSMIndoorMap::MapCSSResultLayer &res);
    void processLink(OSM::Element elem, int floorLevel, LinkDirection linkDir, const KOSMIndoorMap::MapCSSResultLayer &res);

    void addVertex(double x, double y, double z);
    void addFace(std::size_t i, std::size_t j, std::size_t k);
    void addOffMeshConnection(double x1, double y1, double z1, double x2, double y2, double z2, LinkDirection linkDir, AreaType areaType);

    void writeGsetFile();
    void writeObjFile();

    KOSMIndoorMap::MapData m_data;
    KOSMIndoorMap::MapCSSStyle m_style;
    KOSMIndoorMap::MapCSSResult m_filterResult;

    NavMeshTransform m_transform;

    std::unordered_map<OSM::Id, int> m_nodeLevelMap;
    KOSMIndoorMap::AbstractOverlaySource *m_equipmentModel = nullptr;

    // diganostic obj output
    QString m_gsetFileName;
    QString m_objFileName;
    qsizetype m_vertexOffset;
    QByteArray m_objVertices;
    QByteArray m_objFaces;
    QByteArray m_gsetData;
    QBuffer m_objVertexBuffer;
    QBuffer m_objFaceBuffer;
    QBuffer m_gsetBuffer;
};
}

#endif
