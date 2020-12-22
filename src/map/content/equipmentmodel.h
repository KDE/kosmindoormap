/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORMAP_EQUIPMENTMODEL_H
#define KOSMINDOORMAP_EQUIPMENTMODEL_H

#include "kosmindoormap_export.h"

#include <KOSMIndoorMap/MapData>
#include <KOSMIndoorMap/OverlaySource>

namespace KOSMIndoorMap {

/** Elevator or escalator element. */
class KOSMINDOORMAP_EXPORT Equipment
{
public:
    enum Type { Elevator, Escalator };
    std::vector<OSM::Element> sourceElements;
    std::vector<int> levels;
    OSM::UniqueElement syntheticElement;
    Type type;
    // TODO realtime state

    float distanceTo(const OSM::DataSet &dataSet, float lat, float lon) const;
};

/** Overlay source for elevators and escalators. */
class KOSMINDOORMAP_EXPORT EquipmentModel : public AbstractOverlaySource
{
    Q_OBJECT
    Q_PROPERTY(KOSMIndoorMap::MapData mapData READ mapData WRITE setMapData NOTIFY mapDataChanged)
public:
    explicit EquipmentModel(QObject *parent = nullptr);
    ~EquipmentModel();

    MapData mapData() const;
    void setMapData(const MapData &data);

    void forEach(int floorLevel, const std::function<void(OSM::Element, int)> &func) const override;
    void hiddenElements(std::vector<OSM::Element> &elems) const override;

Q_SIGNALS:
    void mapDataChanged();

protected:
    MapData m_data;
    std::vector<Equipment> m_equipment;

    struct {
        OSM::TagKey building;
        OSM::TagKey buildling_part;
        OSM::TagKey conveying;
        OSM::TagKey elevator;
        OSM::TagKey highway;
        OSM::TagKey indoor;
        OSM::TagKey level;
        OSM::TagKey room;
        OSM::TagKey stairwell;

        OSM::TagKey mxoid;
        OSM::TagKey realtimeStatus;
    } m_tagKeys;

private:
    void findEquipment();
};

}

#endif // KOSMINDOORMAP_EQUIPMENTMODEL_H
