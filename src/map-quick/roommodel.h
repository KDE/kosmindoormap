/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORMAP_ROOMMODEL_H
#define KOSMINDOORMAP_ROOMMODEL_H

#include <KOSMIndoorMap/MapData>
#include <KOSMIndoorMap/MapCSSStyle>

#include <KOSM/Element>

#include <QAbstractListModel>
#include <QPolygonF>

namespace KOSMIndoorMap {

/** List all rooms of buildings in a given data set. */
class RoomModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(KOSMIndoorMap::MapData mapData READ mapData WRITE setMapData NOTIFY mapDataChanged)
    /** Number of buildings found in the model data. */
    Q_PROPERTY(int buildingCount READ buildingCount NOTIFY populated)
    /** Returns @c true if there are no rooms in the current map data.
     *  @note Binding to this will disable lazy model population.
     */
    Q_PROPERTY(bool isEmpty READ isEmpty NOTIFY mapDataChanged)

public:
    explicit RoomModel(QObject *parent = nullptr);
    ~RoomModel();

    [[nodiscard]] MapData mapData() const;
    void setMapData(const MapData &data);

    enum Role {
        NameRole = Qt::DisplayRole, ///< room name, if set
        CoordinateRole = Qt::UserRole,
        NumberRole, ///< room number, if set
        LevelRole, ///< numeric level for positioning rather than for display
        ElementRole, ///< OSM element for this room
        TypeNameRole, ///< Type of the room as translated human readable text, if set
        BuildingNameRole, ///< Name of the building the room is in
        LevelLongNameRole, ///< Name of the floor the room is on (long form, if available)
        LevelShortNameRole, ///< Name of the floor the room is on (short form, if available)
    };
    Q_ENUM(Role)

    [[nodiscard]] int rowCount(const QModelIndex &parent = {}) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    [[nodiscard]] int buildingCount() const;
    [[nodiscard]] bool isEmpty() const;

    /** Tries to identify the given room name or number and returns the row index if found. */
    Q_INVOKABLE [[nodiscard]] int findRoom(const QString &name) const;

Q_SIGNALS:
    void mapDataChanged();
    void populated();

private:
    struct Level {
        OSM::Element element;
        int level;
    };

    struct Building {
        OSM::Element element;
        QPolygonF outerPath;
        std::vector<Level> levels;
        int roomCount = 0;
    };

    struct Room {
        OSM::Element element;
        OSM::Element buildingElement;
        OSM::Element levelElement;
        int level;
    };

    void ensurePopulated() const;
    void populateModel();

    MapData m_data;
    MapCSSStyle m_style;

    std::vector<Building> m_buildings;
    std::vector<Room> m_rooms;

    OSM::Languages m_langs;
};

}

#endif // KOSMINDOORMAP_ROOMMODEL_H
