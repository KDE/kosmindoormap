/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "navmeshbuilder.h"
#include "recastsettings_p.h"
#include <logging.h>

#include <KOSMIndoorMap/MapCSSParser>
#include <KOSMIndoorMap/OverlaySource>

#include <loader/levelparser_p.h>
#include <scene/penwidthutil_p.h>
#include <scene/scenegraphitem.h>
#include <style/mapcssdeclaration_p.h>
#include <style/mapcssstate_p.h>

#include <QBuffer>
#include <QFile>
#include <QPolygonF>
#include <QPainterPath>

#include <private/qtriangulator_p.h>
#include <private/qtriangulatingstroker_p.h>

using namespace KOSMIndoorRouting;

//BEGIN TODO largely copied from SceneController, refactor/unify?
static QPolygonF createPolygon(const OSM::DataSet &dataSet, OSM::Element e)
{
    const auto path = e.outerPath(dataSet);
    if (path.empty()) {
        return {};
    }

    QPolygonF poly;
    // Element::outerPath takes care of re-assembling broken up line segments
    // the below takes care of properly merging broken up polygons
    for (auto it = path.begin(); it != path.end();) {
        QPolygonF subPoly;
        subPoly.reserve(path.size());
        OSM::Id pathBegin = (*it)->id;

        auto subIt = it;
        for (; subIt != path.end(); ++subIt) {
            subPoly.push_back(QPointF((*subIt)->coordinate.lonF(), (*subIt)->coordinate.latF()));
            if ((*subIt)->id == pathBegin && subIt != it && subIt != std::prev(path.end())) {
                ++subIt;
                break;
            }
        }
        it = subIt;
        poly = poly.isEmpty() ? std::move(subPoly) : poly.united(subPoly);
    }
    return poly;
}

// @see https://wiki.openstreetmap.org/wiki/Relation:multipolygon
static QPainterPath createPath(const OSM::DataSet &dataSet, const OSM::Element e)
{
    assert(e.type() == OSM::Type::Relation);
    QPolygonF outerPath = createPolygon(dataSet, e); // TODO this is actually not correct for the multiple outer polygon case
    QPainterPath path;
    path.setFillRule(Qt::OddEvenFill);

    for (const auto &mem : e.relation()->members) {
        const bool isInner = std::strcmp(mem.role().name(), "inner") == 0;
        const bool isOuter = std::strcmp(mem.role().name(), "outer") == 0;
        if (mem.type() != OSM::Type::Way || (!isInner && !isOuter)) {
            continue;
        }
        if (auto way = dataSet.way(mem.id)) {
            const auto subPoly = createPolygon(dataSet, OSM::Element(way));
            if (subPoly.isEmpty()) {
                continue;
            }
            path.addPolygon(subPoly);
            path.closeSubpath();
        }
    }

    return path;
}
//END

NavMeshBuilder::NavMeshBuilder(QObject *parent)
    : QObject(parent)
{
}

NavMeshBuilder::~NavMeshBuilder() = default;

void NavMeshBuilder::setMapData(const KOSMIndoorMap::MapData &mapData)
{
    m_data = mapData;

    if (m_style.isEmpty()) {
        KOSMIndoorMap::MapCSSParser p;
        m_style = p.parse(QStringLiteral(":/org.kde.kosmindoorrouting/navmesh-filter.mapcss"));
        if (p.hasError()) {
            qWarning() << p.errorMessage();
            return;
        }
    }

    if (!m_data.isEmpty()) {
        m_style.compile(m_data.dataSet());
    }
}

void NavMeshBuilder::setEquipmentModel(KOSMIndoorMap::AbstractOverlaySource *equipmentModel)
{
    m_equipmentModel = equipmentModel;
    // TODO can we do incremental updates when a realtime elevator status changes?
}

void NavMeshBuilder::writeDebugNavMesh(const QString &gsetFile, const QString &objFile)
{
    m_gsetFileName = gsetFile;
    m_objFileName = objFile;
}

static bool isDoor(const OSM::Node *node)
{
    return !OSM::tagValue(*node, "door").isEmpty();
}

int NavMeshBuilder::levelForNode(OSM::Id nodeId) const
{
    const auto it = m_nodeLevelMap.find(nodeId);
    return it != m_nodeLevelMap.end() ? (*it).second : 0;
}

void NavMeshBuilder::addNodeToLevelIndex(OSM::Id nodeId, int level)
{
    auto it = m_nodeLevelMap.find(nodeId);
    if (it == m_nodeLevelMap.end()) {
        m_nodeLevelMap[nodeId] = level;
        return;
    }
    if ((*it).second != level) {
        (*it).second = std::numeric_limits<int>::min();
    }
}

void NavMeshBuilder::indexNodeLevels()
{
    for (const auto &level : m_data.levelMap()) {
        if (level.first.numericLevel() == 0) {
            continue;
        }
        for (const auto elem : level.second) {
            switch (elem.type()) {
                case OSM::Type::Null:
                    Q_UNREACHABLE();
                case OSM::Type::Node:
                    continue;
                case OSM::Type::Way:
                {
                    // TODO ignore multi-level ways
                    const auto lvl = elem.tagValue("level");
                    if (lvl.isEmpty() || lvl.contains(';')) {
                        break;
                    }
                    for (OSM::Id nodeId : elem.way()->nodes) {
                        addNodeToLevelIndex(nodeId, level.first.numericLevel());
                    }
                    break;
                }
                case OSM::Type::Relation:
                    // TODO
                    break;
            }
        }
    }
}

void NavMeshBuilder::start()
{
    qCDebug(Log);
    m_objVertexBuffer.setBuffer(&m_objVertices);
    m_objFaceBuffer.setBuffer(&m_objFaces);
    m_gsetBuffer.setBuffer(&m_gsetData);
    m_objVertexBuffer.open(QIODevice::WriteOnly);
    m_objFaceBuffer.open(QIODevice::WriteOnly);
    m_gsetBuffer.open(QIODevice::WriteOnly);
    m_vertexOffset = 1;

    m_transform.initialize(m_data.boundingBox());
    indexNodeLevels();

    std::vector<OSM::Element> hiddenElements;
    m_equipmentModel->hiddenElements(hiddenElements);
    std::sort(hiddenElements.begin(), hiddenElements.end());

    for (const auto &level : m_data.levelMap()) {
        for (const auto &elem : level.second) {
            if (std::binary_search(hiddenElements.begin(), hiddenElements.end(), elem)) {
                continue;
            }
            processElement(elem, level.first.numericLevel());
        }

        if (level.first.numericLevel() % 10) {
            continue;
        }
        m_equipmentModel->forEach(level.first.numericLevel(), [this](OSM::Element elem, int floorLevel) {
            processElement(elem, floorLevel);
        });
    }


    m_objVertexBuffer.close();
    m_objFaceBuffer.close();
    [[unlikely]] if ( !m_gsetFileName.isEmpty()) {
        writeGsetFile();
        writeObjFile();
    }
}

void NavMeshBuilder::processElement(OSM::Element elem, int floorLevel)
{
    KOSMIndoorMap::MapCSSState filterState;
    filterState.element = elem;
    m_style.evaluate(std::move(filterState), m_filterResult);

    for (const auto &res : m_filterResult.results()) {
        if (res.layerSelector().isNull()) {
            processGeometry(elem, floorLevel, res);
        } else {
            LinkDirection linkDir = LinkDirection::Bidirectional; // TODO use precompiled keys
            if (std::strcmp(res.layerSelector().name(), "link_forward") == 0) {
                linkDir = LinkDirection::Forward;
            } else if (std::strcmp(res.layerSelector().name(), "link_backward") == 0) {
                linkDir = LinkDirection::Backward;
            }
            processLink(elem, floorLevel, linkDir, res);
        }
    }
}

void NavMeshBuilder::processGeometry(OSM::Element elem, int floorLevel, const KOSMIndoorMap::MapCSSResultLayer &res)
{
    if (res.hasAreaProperties()) {
        const auto prop = res.declaration(KOSMIndoorMap::MapCSSProperty::FillOpacity);
        if (prop && prop->doubleValue() > 0.0) {
            QPainterPath path;
            if (elem.type() == OSM::Type::Relation) {
                path = createPath(m_data.dataSet(), elem);
            } else {
                path.addPolygon(createPolygon(m_data.dataSet(), elem));
            }

            QPainterPath p;
            const auto triSet = qTriangulate(m_transform.mapGeoToNav(path));
            qCDebug(Log) << "A" << elem.url() << m_transform.mapGeoToNav(path).boundingRect() << path.elementCount() << triSet.indices.size() << triSet.vertices.size() << m_vertexOffset << floorLevel;

            for (qsizetype i = 0; i < triSet.vertices.size(); i += 2) {
                addVertex(triSet.vertices[i], m_transform.mapHeightToNav(floorLevel), triSet.vertices[i + 1]);
            }
            if (triSet.indices.type() == QVertexIndexVector::UnsignedShort) {
                for (qsizetype i = 0; i <triSet.indices.size(); i += 3) {
                    addFace(*(reinterpret_cast<const uint16_t*>(triSet.indices.data()) + i) + m_vertexOffset,
                            *(reinterpret_cast<const uint16_t*>(triSet.indices.data()) + i + 1) + m_vertexOffset,
                            *(reinterpret_cast<const uint16_t*>(triSet.indices.data()) + i + 2) + m_vertexOffset);
                }
            } else if (triSet.indices.type() == QVertexIndexVector::UnsignedInt) {
                for (qsizetype i = 0; i <triSet.indices.size(); i += 3) {
                    addFace(*(reinterpret_cast<const uint32_t*>(triSet.indices.data()) + i) + m_vertexOffset,
                            *(reinterpret_cast<const uint32_t*>(triSet.indices.data()) + i + 1) + m_vertexOffset,
                            *(reinterpret_cast<const uint32_t*>(triSet.indices.data()) + i + 2) + m_vertexOffset);
                }
            }
            m_vertexOffset += triSet.vertices.size() / 2;
        }
    }

    if (res.hasLineProperties()) {
        const auto prop = res.declaration(KOSMIndoorMap::MapCSSProperty::Width);
        KOSMIndoorMap::Unit dummyUnit;
        if (const auto penWidth = prop ? KOSMIndoorMap::PenWidthUtil::penWidth(elem, prop, dummyUnit) : 0.0; penWidth > 0.0) {
            QPolygonF poly = m_transform.mapGeoToNav(createPolygon(m_data.dataSet(), elem));
            QPainterPath path;
            path.addPolygon(poly);
            QPen pen;
            // TODO join/cap styles
            pen.setCapStyle(Qt::FlatCap);
            pen.setWidthF(penWidth);

            QTriangulatingStroker stroker;
            stroker.process(qtVectorPathForPath(path), pen, {}, {});
            qCDebug(Log) << "L" << elem.url() << stroker.vertexCount() << pen.widthF();

            for (int i = 0; i < stroker.vertexCount(); i += 2) {
                auto l = floorLevel;
                if (poly.size() == 2 && elem.type() == OSM::Type::Way) { // TODO can we generalize this?
                    const auto way = elem.way();
                    const auto l1 = levelForNode(way->nodes.at(0));
                    const auto l2 = levelForNode(way->nodes.at(1));
                    qCDebug(Log) << "  S" << elem.url() << floorLevel << l1 << l2;
                    if (l1 != l2 && l1 != std::numeric_limits<int>::min() && l2 != std::numeric_limits<int>::min()) {
                        QPointF p(*(stroker.vertices() + i), *(stroker.vertices() + i + 1));
                        l = QLineF(poly.at(0), p).length() < QLineF(poly.at(1), p).length() ? l1 : l2;
                    }
                }
                addVertex(*(stroker.vertices() + i), m_transform.mapHeightToNav(l), *(stroker.vertices() + i + 1));
            }
            for (int i = 0; i < stroker.vertexCount() / 2 - 2; ++i) {
                // GL_TRIANGLE_STRIP winding order
                if (i % 2) {
                    addFace(m_vertexOffset + i, m_vertexOffset + i + 1, m_vertexOffset + i + 2);
                } else {
                    addFace(m_vertexOffset + i + 1, m_vertexOffset + i, m_vertexOffset + i + 2);
                }
            }
            m_vertexOffset += stroker.vertexCount() / 2;
        }
    }

    if (res.hasExtrudeProperties()) {
        const auto prop = res.declaration(KOSMIndoorMap::MapCSSProperty::Extrude);
        if (prop && prop->doubleValue() > 0.0) {
            const auto way = elem.outerPath(m_data.dataSet());
            for (std::size_t i = 0; i < way.size() - 1; ++i) {
                if (isDoor(way[i]) || isDoor(way[i + 1])) {
                    continue;
                }
                const auto p1 = m_transform.mapGeoToNav(way[i]->coordinate);
                const auto p2 = m_transform.mapGeoToNav(way[i + 1]->coordinate);
                addVertex(p1.x(), m_transform.mapHeightToNav(floorLevel), p1.y());
                addVertex(p2.x(), m_transform.mapHeightToNav(floorLevel), p2.y());
                addVertex(p1.x(), m_transform.mapHeightToNav(floorLevel + 10), p1.y());
                addVertex(p2.x(), m_transform.mapHeightToNav(floorLevel + 10), p2.y());
                addFace(m_vertexOffset, m_vertexOffset + 1, m_vertexOffset + 2);
                addFace(m_vertexOffset + 1, m_vertexOffset + 3, m_vertexOffset + 2);
                m_vertexOffset += 4;
            }
        }
    }
}

void NavMeshBuilder::processLink(OSM::Element elem, int floorLevel, LinkDirection linkDir, const KOSMIndoorMap::MapCSSResultLayer &res)
{
    if (res.hasAreaProperties()) {
        std::vector<int> levels;
        KOSMIndoorMap::LevelParser::parse(elem.tagValue("level"), elem, [&levels](int level, auto) { levels.push_back(level); });
        if (levels.size() > 1) {
            qDebug() << "E" << elem.url() << levels;
            // TODO doesn't work for concave polygons!
            const QPointF p = m_transform.mapGeoToNav(elem.center());
            for (std::size_t i = 0; i < levels.size() - 1; ++i) {
                addOffMeshConnection(p.x(), m_transform.mapHeightToNav(levels[i]), p.y(), p.x(), m_transform.mapHeightToNav(levels[i + 1]), p.y(), LinkDirection::Bidirectional, AreaType::Elevator); // TODO area type from MapCSS
            }
        }
    }
    if (res.hasLineProperties() && elem.type() == OSM::Type::Way) {
        const auto way = elem.way();
        if (way->nodes.size() == 2) {
            const auto l1 = levelForNode(way->nodes.at(0));
            const auto l2 = levelForNode(way->nodes.at(1));
            qCDebug(Log) << "  LINK" << elem.url() << floorLevel << l1 << l2;
            if (l1 != l2 && l1 != std::numeric_limits<int>::min() && l2 != std::numeric_limits<int>::min()) {
                const auto poly = createPolygon(m_data.dataSet(), elem);
                const auto p1 = m_transform.mapGeoToNav(poly.at(0));
                const auto p2 = m_transform.mapGeoToNav(poly.at(1));
                addOffMeshConnection(p1.x(), m_transform.mapHeightToNav(l1), p1.y(), p2.x(), m_transform.mapHeightToNav(l2), p2.y(), linkDir, AreaType::Escalator); // TODO area type from MapCSS
            }
        }
    }
}

void NavMeshBuilder::addVertex(double x, double y, double z)
{
    m_objVertexBuffer.write("v ");
    m_objVertexBuffer.write(QByteArray::number(x));
    m_objVertexBuffer.write(" ");
    m_objVertexBuffer.write(QByteArray::number(y));
    m_objVertexBuffer.write(" ");
    m_objVertexBuffer.write(QByteArray::number(z));
    m_objVertexBuffer.write("\n");
}

void NavMeshBuilder::addFace(std::size_t i, std::size_t j, std::size_t k)
{
    m_objFaceBuffer.write("f ");
    m_objFaceBuffer.write(QByteArray::number(i));
    m_objFaceBuffer.write(" ");
    m_objFaceBuffer.write(QByteArray::number(j));
    m_objFaceBuffer.write(" ");
    m_objFaceBuffer.write(QByteArray::number(k));
    m_objFaceBuffer.write("\n");
}

void NavMeshBuilder::addOffMeshConnection(double x1, double y1, double z1, double x2, double y2, double z2, LinkDirection linkDir, AreaType areaType)
{
    if (linkDir == LinkDirection::Backward) {
        std::swap(x1, x2);
        std::swap(y1, y2);
        std::swap(z1, z2);
        linkDir = LinkDirection::Forward;
    }

    m_gsetBuffer.write("c ");
    m_gsetBuffer.write(QByteArray::number(x1));
    m_gsetBuffer.write(" ");
    m_gsetBuffer.write(QByteArray::number(y1));
    m_gsetBuffer.write(" ");
    m_gsetBuffer.write(QByteArray::number(z1));
    m_gsetBuffer.write("  ");
    m_gsetBuffer.write(QByteArray::number(x2));
    m_gsetBuffer.write(" ");
    m_gsetBuffer.write(QByteArray::number(y2));
    m_gsetBuffer.write(" ");
    m_gsetBuffer.write(QByteArray::number(z2));
    // radius
    m_gsetBuffer.write(" 0.6 ");
    m_gsetBuffer.write(linkDir == LinkDirection::Bidirectional ? "1" : "0");
    // area id, flags
    m_gsetBuffer.write(" ");
    m_gsetBuffer.write(QByteArray::number(qToUnderlying(areaType)));
    m_gsetBuffer.write(" 8\n");
}

void NavMeshBuilder::writeGsetFile()
{
    QFile f(m_gsetFileName);
    f.open(QFile::WriteOnly);
    f.write("f ");
    f.write(m_objFileName.toUtf8());
    f.write("\n");

    f.write("s ");
    f.write(QByteArray::number(RECAST_CELL_SIZE));
    f.write(" ");
    f.write(QByteArray::number(RECAST_CELL_HEIGHT));
    f.write(" ");

    f.write(QByteArray::number(RECAST_AGENT_HEIGHT));
    f.write(" ");
    f.write(QByteArray::number(RECAST_AGENT_RADIUS));
    f.write(" ");
    f.write(QByteArray::number(RECAST_AGENT_MAX_CLIMB));
    f.write(" ");
    f.write(QByteArray::number(RECAST_AGENT_MAX_SLOPE));
    f.write(" ");

    f.write("8 20 "); // region min/merged size ??
    f.write("12 1.3 6 "); // edge max len/max error, vertex per polygon ??
    f.write("6 1 "); // details sample dist/max error ??
    f.write(QByteArray::number(qToUnderlying(RECAST_PARTITION_TYPE))); // partition type
    f.write(" ");

    // bbox min
    auto p = m_transform.mapGeoToNav(m_data.boundingBox().min);
    f.write(QByteArray::number(p.x()));
    f.write(" ");
    f.write(QByteArray::number(std::prev(m_data.levelMap().end())->first.numericLevel()));
    f.write(" ");
    f.write(QByteArray::number(p.y()));
    f.write(" ");

    // bbox max
    p = m_transform.mapGeoToNav(m_data.boundingBox().max);
    f.write(QByteArray::number(p.x()));
    f.write(" ");
    f.write(QByteArray::number(m_data.levelMap().begin()->first.numericLevel()));
    f.write(" ");
    f.write(QByteArray::number(p.y()));
    f.write(" ");

    f.write("0\n"); // tile size?

    f.write(m_gsetData);
}

void NavMeshBuilder::writeObjFile()
{
    QFile f(m_objFileName);
    f.open(QFile::WriteOnly);
    f.write(m_objVertices);
    f.write(m_objFaces);
}
