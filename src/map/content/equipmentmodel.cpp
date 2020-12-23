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

    emit update();
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
    // TODO
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
        if ((highway == "footway" || highway == "steps") && (!conveying.isEmpty() && conveying != "no")) {
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
            // TODO single level elevators need to be merged with duplicates on other levels
            Equipment elevator;
            elevator.type = Equipment::Elevator;
            elevator.sourceElements.push_back(e);
            LevelParser::parse(e.tagValue(m_tagKeys.level), e, [&elevator](int level, OSM::Element) {
                elevator.levels.push_back(level);
            });
            if (elevator.levels.empty()) {
                elevator.levels.push_back(0);
            }
            m_equipment.push_back(std::move(elevator));
        }

    }, OSM::IncludeNodes | OSM::IncludeWays);
}
