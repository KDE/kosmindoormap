/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "equipmentmodel.h"
#include "../loader/levelparser_p.h"

#include <osm/geomath.h>

#include <QDebug>

using namespace KOSMIndoorMap;

float Equipment::distanceTo(const OSM::DataSet &dataSet, float lat, float lon) const
{
    if (sourceElements.empty()) {
        return std::numeric_limits<float>::max();
    }

    switch (sourceElements[0].type()) {
        case OSM::Type::Null:
            return std::numeric_limits<float>::max();
        case OSM::Type::Node:
            return OSM::distance(sourceElements[0].center(), OSM::Coordinate(lat, lon));
        case OSM::Type::Way:
        case OSM::Type::Relation:
        {
            const auto path = sourceElements[0].outerPath(dataSet);
            return OSM::distance(path, OSM::Coordinate(lat, lon));
        }
    }
    Q_UNREACHABLE();
    return std::numeric_limits<float>::max();
}


EquipmentModel::EquipmentModel(QObject* parent)
    : AbstractOverlaySource(nullptr, parent)
{
}

EquipmentModel::~EquipmentModel() = default;

MapData EquipmentModel::mapData() const
{
    return m_data;
}

void EquipmentModel::setMapData(const MapData &data)
{
    if (m_data == data) {
        return;
    }

    m_equipment.clear();
    m_data = data;

    if (!m_data.isEmpty()) {
        m_tagKeys.building = m_data.dataSet().tagKey("building");
        m_tagKeys.buildling_part = m_data.dataSet().tagKey("building:part");
        m_tagKeys.conveying = m_data.dataSet().tagKey("conveying");
        m_tagKeys.elevator = m_data.dataSet().tagKey("elevator");
        m_tagKeys.highway = m_data.dataSet().tagKey("highway");
        m_tagKeys.indoor = m_data.dataSet().tagKey("indoor");
        m_tagKeys.level = m_data.dataSet().tagKey("level");
        m_tagKeys.room = m_data.dataSet().tagKey("room");
        m_tagKeys.stairwell = m_data.dataSet().tagKey("stairwell");

        m_tagKeys.mxoid = m_data.dataSet().makeTagKey("mx:oid");
        m_tagKeys.realtimeStatus = m_data.dataSet().makeTagKey("mx:realtime_status");
        findEquipment();
    }

    for (const auto &eq : m_equipment) {
        qDebug() << "  E" << eq.sourceElements.size() << eq.levels << eq.type;
    }

    Q_EMIT update();
}

void EquipmentModel::forEach(int floorLevel, const std::function<void (OSM::Element, int)> &func) const
{
    for (const auto &eq : m_equipment) {
        if (!eq.syntheticElement || std::find(eq.levels.begin(), eq.levels.end(), floorLevel) == eq.levels.end()) {
            continue;
        }
        func(eq.syntheticElement, floorLevel);
    }
}

void EquipmentModel::hiddenElements(std::vector<OSM::Element> &elems) const
{
    for (const auto &eq : m_equipment) {
        if (!eq.syntheticElement) {
            continue;
        }
        elems.insert(elems.end(), eq.sourceElements.begin(), eq.sourceElements.end());
    }
}

static bool isConveying(const QByteArray &b)
{
    return b == "yes" || b == "forward" || b == "backward" || b == "reversible";
}

void EquipmentModel::findEquipment()
{
    OSM::for_each(m_data.dataSet(), [this](OSM::Element e) {
        if (!e.hasTags()) {
            return;
        }

        // escalators
        const auto highway = e.tagValue(m_tagKeys.highway);
        const auto conveying = e.tagValue(m_tagKeys.conveying);
        if ((highway == "footway" || highway == "steps") && isConveying(conveying)) {
            Equipment escalator;
            escalator.type = Equipment::Escalator;
            escalator.sourceElements.push_back(e);
            LevelParser::parse(e.tagValue(m_tagKeys.level), e, [&escalator](int level, OSM::Element) {
                escalator.levels.push_back(level);
            });
            m_equipment.push_back(std::move(escalator));
        }

        // elevators
        const auto building = e.tagValue(m_tagKeys.building);
        const auto buildling_part = e.tagValue(m_tagKeys.buildling_part);
        const auto elevator = e.tagValue(m_tagKeys.elevator);
        const auto indoor = e.tagValue(m_tagKeys.indoor);
        const auto room = e.tagValue(m_tagKeys.room);
        const auto stairwell = e.tagValue(m_tagKeys.stairwell);

        if (building == "elevator"
         || buildling_part == "elevator" || (buildling_part == "yes" && elevator == "yes")
         || highway == "elevator"
         || room == "elevator"
         || stairwell == "elevator"
         || (indoor == "room" && elevator == "yes"))
        {
            Equipment elevator;
            elevator.type = Equipment::Elevator;
            elevator.sourceElements.push_back(e);
            LevelParser::parse(e.tagValue(m_tagKeys.level), e, [&elevator](int level, OSM::Element) {
                elevator.levels.push_back(level);
            });
            if (elevator.levels.empty()) {
                elevator.levels.push_back(0);
            }

            // try to find duplicate elements on other levels
            for (auto &e : m_equipment) {
                if (e.type != Equipment::Elevator) {
                    continue;
                }
                if (OSM::intersects(e.sourceElements[0].boundingBox(), elevator.sourceElements[0].boundingBox())) {
                    // TODO check for non-intersecting but present level sets?
                    qDebug() << "merging elevator elements:" << elevator.sourceElements[0].url() << e.sourceElements[0].url();
                    e.sourceElements.push_back(elevator.sourceElements[0]);
                    e.levels.insert(e.levels.end(), elevator.levels.begin(), elevator.levels.end());
                    return;
                }
            }

            m_equipment.push_back(std::move(elevator));
        }

    }, OSM::IncludeNodes | OSM::IncludeWays);

    // finalize elevator merging
    for (auto &elevator : m_equipment) {
        if (elevator.type != Equipment::Elevator || elevator.sourceElements.size() < 2) {
            continue;
        }

        std::sort(elevator.levels.begin(), elevator.levels.end());
        elevator.levels.erase(std::unique(elevator.levels.begin(), elevator.levels.end()), elevator.levels.end());
        if (elevator.levels.size() < 2) {
            continue;
        }

        std::sort(elevator.sourceElements.begin(), elevator.sourceElements.end(), [](auto lhs, auto rhs) { return lhs.type() > rhs.type(); });
        createSyntheticElement(elevator);
    }
}

void EquipmentModel::createSyntheticElement(Equipment& eq) const
{
    if (eq.syntheticElement) {
        return;
    }

    eq.syntheticElement = OSM::copy_element(eq.sourceElements[0]);
    eq.syntheticElement.setTagValue(m_tagKeys.mxoid, QByteArray::number((qlonglong)eq.syntheticElement.element().id()));
    eq.syntheticElement.setId(m_data.dataSet().nextInternalId());

    // clone tags
    for (auto it = std::next(eq.sourceElements.begin()); it != eq.sourceElements.end(); ++it) {
        for (auto tagIt = (*it).tagsBegin(); tagIt != (*it).tagsEnd(); ++tagIt) {
            if ((*tagIt).key == m_tagKeys.level) {
                continue;
            }

            if (eq.syntheticElement.element().hasTag((*tagIt).key)) {
                // ### for testing only
                if (eq.syntheticElement.element().tagValue((*tagIt).key) != (*tagIt).value) {
                    qDebug() << "  tag value conflict:" << (*tagIt).key.name() << (*tagIt).value << eq.sourceElements[0].url() << eq.syntheticElement.element().tagValue((*tagIt).key);
                }
                continue;
            }
            eq.syntheticElement.setTagValue((*tagIt).key, (*tagIt).value);
        }
    }

    if (eq.levels.size() > 1) {
        auto levelValue = QByteArray::number(eq.levels.at(0) / 10);
        for (auto it = std::next(eq.levels.begin()); it != eq.levels.end(); ++it) {
            levelValue += ';' + QByteArray::number((*it) / 10);
        }
        eq.syntheticElement.setTagValue(m_tagKeys.level, levelValue);
    }
}
