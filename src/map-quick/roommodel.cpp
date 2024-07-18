/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "roommodel.h"
#include "localization.h"
#include "logging.h"
#include "osmelement.h"

#include <style/mapcssdeclaration_p.h>
#include <style/mapcssstate_p.h>

#include <KOSMIndoorMap/MapCSSParser>
#include <KOSMIndoorMap/MapCSSResult>

#include <KLocalizedString>

#include <QDebug>
#include <QFile>
#include <QPointF>

#include <limits>

using namespace KOSMIndoorMap;

RoomModel::RoomModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_langs(OSM::Languages::fromQLocale(QLocale()))
{
}

RoomModel::~RoomModel() = default;

MapData RoomModel::mapData() const
{
    return m_data;
}

void RoomModel::setMapData(const MapData &data)
{
    if (m_data == data) {
        return;
    }

    if (m_style.isEmpty()) {
        MapCSSParser p;
        m_style = p.parse(QStringLiteral(":/org.kde.kosmindoormap/assets/quick/room-model.mapcss"));
        if (p.hasError()) {
            qWarning() << p.errorMessage();
            return;
        }
    }

    beginResetModel();
    m_buildings.clear();
    m_rooms.clear();
    m_data = data;
    if (!m_data.isEmpty()) {
        m_style.compile(m_data.dataSet());
    }
    endResetModel();
    Q_EMIT mapDataChanged();
}

int RoomModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    ensurePopulated();
    return (int)m_rooms.size();
}

QVariant RoomModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index)) {
        return {};
    }

    const auto &room = m_rooms[index.row()];
    switch (role) {
        case NameRole:
            // TODO better name/number handling - separate roles?
            return QString::fromUtf8(room.element.tagValue(m_langs, "name"));
        case NumberRole:
            return QString::fromUtf8(room.element.tagValue("ref"));
        case TypeNameRole:
        {
            const auto types = room.element.tagValue("room", "amenity").split(';');
            QStringList l;
            for (const auto &type : types) {
                if (type == "yes") {
                    continue;
                }
                auto s = Localization::amenityType(type.trimmed().constData(), Localization::ReturnEmptyOnUnknownKey);
                if (!s.isEmpty()) {
                    l.push_back(std::move(s));
                }
            }
            return QLocale().createSeparatedList(l);
        }
        case CoordinateRole:
        {
            const auto center = room.element.center();
            return QPointF(center.lonF(), center.latF());
        }
        case LevelRole:
            return room.level;
        case ElementRole:
            return QVariant::fromValue(OSMElement(room.element));
        case BuildingNameRole:
            return QString::fromUtf8(room.buildingElement.tagValue(m_langs, "name", "local_ref", "ref"));
        case LevelLongNameRole:
        {
            auto s = QString::fromUtf8(room.levelElement.tagValue(m_langs, "name", "level:ref"));
            if (!s.isEmpty()) {
                return s;
            }

            if ((room.level / 10) == 0) {
                return i18n("Ground floor");
            }
            return i18n("Floor %1", room.level / 10); // TODO this isn't properly localized...
        }
        case LevelShortNameRole:
        {
            auto s = QString::fromUtf8(room.levelElement.tagValue(m_langs, "level:ref"));
            if (!s.isEmpty()) {
                return s;
            }
            return QString::number(room.level/ 10); // TODO this could use localized floor level abbrevations
        }

    }

    return {};
}

QHash<int, QByteArray> RoomModel::roleNames() const
{
    auto r = QAbstractListModel::roleNames();
    r.insert(NameRole, "name");
    r.insert(NumberRole, "number");
    r.insert(TypeNameRole, "typeName");
    r.insert(CoordinateRole, "coordinate");
    r.insert(LevelRole, "level");
    r.insert(ElementRole, "element");
    r.insert(BuildingNameRole, "buildingName");
    r.insert(LevelLongNameRole, "levelLongName");
    r.insert(LevelShortNameRole, "levelShortName");
    return r;
}

int RoomModel::buildingCount() const
{
    return (int)m_buildings.size();
}

bool RoomModel::isEmpty() const
{
    return rowCount() == 0;
}

void RoomModel::ensurePopulated() const
{
    if (m_rooms.empty() && !m_data.isEmpty()) {
        // we assume that this is expensive but almost never will result in an empty result
        // and if it does nevertheless, it's a sparsely populated tile where this is cheap
        const_cast<RoomModel*>(this)->populateModel();
    }
}

void RoomModel::populateModel()
{
    // find all buildings
    const auto buildingKey = m_data.dataSet().tagKey("building");
    const auto nameKey = m_data.dataSet().tagKey("name");
    const auto refKey = m_data.dataSet().tagKey("ref");

    for (auto it = m_data.levelMap().begin(); it != m_data.levelMap().end(); ++it) {
        for (const auto &e : (*it).second) {
            if (e.type() == OSM::Type::Node || !OSM::contains(m_data.boundingBox(), e.center())) {
                continue;
            }
            if (e.hasTag(buildingKey) && (e.hasTag(nameKey) || e.hasTag(refKey))) {
                Building building;
                building.element = e;
                // building.outerPath = e.outerPath(m_data.dataSet()); TODO needed?
                m_buildings.push_back(std::move(building));
            }
        }
    }

    // find floor levels for each building
    const auto indoorKey = m_data.dataSet().tagKey("indoor");
    for (auto it = m_data.levelMap().begin(); it != m_data.levelMap().end(); ++it) {
        for (const auto &e : (*it).second) {
            if (e.type() == OSM::Type::Node || !OSM::contains(m_data.boundingBox(), e.center())) {
                continue;
            }
            if (e.tagValue(indoorKey) == "level") {
                Level level;
                level.element = e;
                level.level = (*it).first.numericLevel();

                // find building this level belongs to
                for (auto &building : m_buildings) {
                    // TODO this is likely not precise enough?
                    if (OSM::intersects(e.boundingBox(), building.element.boundingBox())) {
                        building.levels.push_back(level);
                        break;
                    }
                }
            }
        }
    }

    // find all rooms
    MapCSSResult filterResult;
    for (auto it = m_data.levelMap().begin(); it != m_data.levelMap().end(); ++it) {
        for (const auto &e : (*it).second) {
            if (e.type() == OSM::Type::Node || !OSM::contains(m_data.boundingBox(), e.center())) {
                continue;
            }

            MapCSSState filterState;
            filterState.element = e;
            m_style.initializeState(filterState);
            m_style.evaluate(filterState, filterResult);

            const auto &res = filterResult[{}];
            if (auto prop = res.declaration(MapCSSProperty::Opacity); !prop || prop->doubleValue() < 1.0) {
                continue; // hidden element
            }

            Room room;
            room.element = e;
            room.level = (*it).first.numericLevel(); // TODO we only need one entry, not one per level!

            // find the building this room is in
            for (auto &building :m_buildings) {
                // TODO this is likely not precise enough?
                if (OSM::intersects(e.boundingBox(), building.element.boundingBox())) {
                    room.buildingElement = building.element;
                    ++building.roomCount;

                    // find level meta-data if available
                    for (const auto &level : building.levels) {
                        if (level.level == room.level) {
                            room.levelElement = level.element;
                            break;
                        }
                    }

                    break;
                }
            }

            m_rooms.push_back(std::move(room));
        }
    }

    // TODO we could accumulate the covered levels and show all of them?
    // de-duplicate multi-level entries
    // we could also just iterate over the non-level-split data, but
    // then we need to reparse the level data here...
    std::sort(m_rooms.begin(), m_rooms.end(), [](const auto &lhs, const auto &rhs) {
        if (lhs.element == rhs.element) {
            return std::abs(lhs.level) < std::abs(rhs.level);
        }
        return lhs.element < rhs.element;
    });
    m_rooms.erase(std::unique(m_rooms.begin(), m_rooms.end(), [](const auto &lhs, const auto &rhs) {
        return lhs.element == rhs.element;
    }), m_rooms.end());

    // de-duplicate multi-level rooms that consist of multiple OSM elements (e.g. due to varying sizes per floor)
    // TODO

    // sort by building
    std::sort(m_rooms.begin(), m_rooms.end(), [](const auto &lhs, const auto &rhs) {
        return lhs.buildingElement < rhs.buildingElement;
    });

    // remove buildings without rooms
    m_buildings.erase(std::remove_if(m_buildings.begin(), m_buildings.end(), [](const auto &b) { return b.roomCount == 0; }), m_buildings.end());

    qCDebug(Log) << m_buildings.size() << "buildings found";
    qCDebug(Log) << m_rooms.size() << "rooms found";
    Q_EMIT populated();
}

int RoomModel::findRoom(const QString &name) const
{
    if (name.isEmpty()) {
        return -1;
    }

    ensurePopulated();
    for (auto it = m_rooms.begin(); it != m_rooms.end(); ++it) {
        // TODO match room numbers, space-ignoring fuzzy match, unambiguous substring matching
        if (QUtf8StringView((*it).element.tagValue("name")).compare(name, Qt::CaseInsensitive) == 0) {
            return (int)std::distance(m_rooms.begin(), it);
        }
    }

    return -1;
}

#include "moc_roommodel.cpp"
