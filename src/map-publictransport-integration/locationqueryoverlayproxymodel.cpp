/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "locationqueryoverlayproxymodel.h"

#include <KPublicTransport/Location>
#include <KPublicTransport/LocationQueryModel>
#include <KPublicTransport/RentalVehicle>

#include <osm/element.h>
#include <osm/geomath.h>

using namespace KOSMIndoorMap;
using namespace KPublicTransport;

struct vehicle_type {
    const char *tagName;
    RentalVehicle::VehicleType vehicleType;
};
static constexpr const vehicle_type vehicle_type_map[] = {
    { "mx:realtime_available:bike", RentalVehicle::Bicycle },
    { "mx:realtime_available:pedelec", RentalVehicle::Pedelec },
    { "mx:realtime_available:scooter", RentalVehicle::ElectricKickScooter },
    { "mx:realtime_available:motorcycle", RentalVehicle::ElectricMoped },
    { "mx:realtime_available:car", RentalVehicle::Car },
};

LocationQueryOverlayProxyModel::LocationQueryOverlayProxyModel(QObject *parent)
    : QAbstractListModel(parent)
{
    static_assert((sizeof(vehicle_type_map) / sizeof(vehicle_type)) == (sizeof(LocationQueryOverlayProxyModel::m_realtimeAvailableTagKeys) / sizeof(OSM::TagKey)));
}

LocationQueryOverlayProxyModel::~LocationQueryOverlayProxyModel() = default;

MapData LocationQueryOverlayProxyModel::mapData() const
{
    return m_data;
}

void LocationQueryOverlayProxyModel::setMapData(const MapData &data)
{
    if (m_data == data) {
        return;
    }

    beginResetModel();
    m_data = data;

    if (!m_data.isEmpty()) {
        m_tagKeys.name = m_data.dataSet().makeTagKey("name");
        m_tagKeys.amenity = m_data.dataSet().makeTagKey("amenity");
        m_tagKeys.capacity = m_data.dataSet().makeTagKey("capacity");
        m_tagKeys.realtimeAvailable = m_data.dataSet().makeTagKey("mx:realtime_available");
        m_tagKeys.network = m_data.dataSet().makeTagKey("network");
        m_tagKeys.mxoid = m_data.dataSet().makeTagKey("mx:oid");
        m_tagKeys.remainingRange = m_data.dataSet().makeTagKey("mx:remaining_range");
        m_tagKeys.vehicle = m_data.dataSet().makeTagKey("mx:vehicle");
        m_tagKeys.addr_street = m_data.dataSet().makeTagKey("addr:street");
        m_tagKeys.addr_city = m_data.dataSet().makeTagKey("addr:city");
        m_tagKeys.addr_postcode = m_data.dataSet().makeTagKey("addr:postcode");
    }

    int i = 0;
    for (const auto &v : vehicle_type_map) {
        m_realtimeAvailableTagKeys[i++] = m_data.dataSet().makeTagKey(v.tagName);
    }

    initialize();
    endResetModel();
    Q_EMIT mapDataChanged();
}

QObject* LocationQueryOverlayProxyModel::sourceModel() const
{
    return m_sourceModel;
}

void LocationQueryOverlayProxyModel::setSourceModel(QObject *sourceModel)
{
    if (m_sourceModel == sourceModel) {
        return;
    }
    beginResetModel();
    m_sourceModel = qobject_cast<QAbstractItemModel*>(sourceModel);
    initialize();
    endResetModel();

    connect(m_sourceModel, &QAbstractItemModel::modelReset, this, [this]() {
        beginResetModel();
        initialize();
        endResetModel();
    });
    connect(m_sourceModel, &QAbstractItemModel::rowsInserted, this, [this](const QModelIndex &parent, int first, int last) {
        if (parent.isValid() || m_data.isEmpty()) {
            return;
        }
        beginInsertRows({}, first, last);
        for (int i = first; i <= last; ++i) {
            m_nodes.insert(m_nodes.begin() + i, nodeForRow(i));
        }
        endInsertRows();
    });
    connect(m_sourceModel, &QAbstractItemModel::rowsRemoved, this, [this](const QModelIndex &parent, int first, int last) {
        if (parent.isValid() || m_data.isEmpty()) {
            return;
        }
        beginRemoveRows({}, first, last);
        m_nodes.erase(m_nodes.begin() + first, m_nodes.begin() + last);
        endRemoveRows();
    });
    connect(m_sourceModel, &QAbstractItemModel::dataChanged, this, [this](const QModelIndex &first, const QModelIndex &last) {
        if (first.parent().isValid() || last.parent().isValid() || m_data.isEmpty()) {
            return;
        }
        for (int i = first.row(); i <= last.row(); ++i) {
            m_nodes[i] = nodeForRow(i);
        }
        Q_EMIT dataChanged(index(first.row(), 0), index(last.row(), 0));
    });
}

int LocationQueryOverlayProxyModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_nodes.size();
}

QVariant LocationQueryOverlayProxyModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }

    switch (role) {
        case ElementRole:
            return QVariant::fromValue(OSM::Element(&m_nodes[index.row()].overlayNode));
        case LevelRole:
            return 0;
        case HiddenElementRole:
            return QVariant::fromValue(m_nodes[index.row()].sourceElement);
    }

    return {};
}

QHash<int, QByteArray> LocationQueryOverlayProxyModel::roleNames() const
{
    auto n = QAbstractListModel::roleNames();
    n.insert(ElementRole, "osmElement");
    n.insert(LevelRole, "level");
    n.insert(HiddenElementRole, "hiddenElement");
    return n;
}

void LocationQueryOverlayProxyModel::initialize()
{
    if (m_data.isEmpty() || !m_sourceModel) {
        return;
    }

    m_nodes.clear();
    const auto rows = m_sourceModel->rowCount();
    m_nodes.reserve(rows);
    for (int i = 0; i < rows; ++i) {
        m_nodes.push_back(nodeForRow(i));
    }
}

static void setTagIfMissing(OSM::Node &node, OSM::TagKey tag, const QString &value)
{
    if (OSM::tagValue(node, tag).isEmpty() && !value.isEmpty()) {
        OSM::setTagValue(node, tag, value.toUtf8());
    }
}

LocationQueryOverlayProxyModel::Info LocationQueryOverlayProxyModel::nodeForRow(int row) const
{
    const auto idx = m_sourceModel->index(row, 0);
    const auto loc = idx.data(LocationQueryModel::LocationRole).value<Location>();

    Info info;
    info.overlayNode.coordinate = OSM::Coordinate(loc.latitude(), loc.longitude());

    switch (loc.type()) {
        case Location::Place:
        case Location::Stop:
        case Location::CarpoolPickupDropoff:
            qDebug() << "got a location type we didn't ask for:" << loc.type() << loc.name();
            break;
        case Location::RentedVehicleStation:
        {
            const auto station = loc.rentalVehicleStation();

            // try to find a matching node in the base OSM data
            for (const auto &n : m_data.dataSet().nodes) {
                if (OSM::distance(n.coordinate, info.overlayNode.coordinate) < 10 && OSM::tagValue(n, m_tagKeys.amenity) == "bicycle_rental") {
                    qDebug() << "found matching node, cloning that!" << n.url();
                    info.sourceElement = OSM::Element(&n);
                    info.overlayNode = n;
                    OSM::setTagValue(info.overlayNode, m_tagKeys.mxoid, QByteArray::number(qlonglong(n.id)));
                    break;
                }
            }

            info.overlayNode.id = m_data.dataSet().nextInternalId();
            OSM::setTagValue(info.overlayNode, m_tagKeys.amenity, "bicycle_rental");
            if (station.capacity() >= 0) {
                OSM::setTagValue(info.overlayNode, m_tagKeys.capacity, QByteArray::number(station.capacity()));
            }
            OSM::setTagValue(info.overlayNode, m_tagKeys.realtimeAvailable, QByteArray::number(station.availableVehicles()));
            setTagIfMissing(info.overlayNode, m_tagKeys.network, station.network().name());
            setTagIfMissing(info.overlayNode, m_tagKeys.name, loc.name());
            setTagIfMissing(info.overlayNode, m_tagKeys.addr_street, loc.streetAddress());
            setTagIfMissing(info.overlayNode, m_tagKeys.addr_city, loc.locality());
            setTagIfMissing(info.overlayNode, m_tagKeys.addr_postcode, loc.postalCode());

            int i = 0;
            for (const auto &v : vehicle_type_map) {
                if (station.availableVehicles(v.vehicleType) > 0) {
                    OSM::setTagValue(info.overlayNode, m_realtimeAvailableTagKeys[i], QByteArray::number(station.availableVehicles(v.vehicleType)));
                }
                ++i;
            }

            break;
        }
        case Location::RentedVehicle:
        {
            const auto vehicle = loc.data().value<RentalVehicle>();

            // free floating vehicles have no matching OSM element, so no point in searching for one
            info.overlayNode.id = m_data.dataSet().nextInternalId();
            switch (vehicle.type()) {
                case RentalVehicle::Unknown:
                case RentalVehicle::Bicycle:
                case RentalVehicle::Pedelec:
                    OSM::setTagValue(info.overlayNode, m_tagKeys.vehicle, "bicycle_rental");
                    break;
                case RentalVehicle::ElectricKickScooter:
                    OSM::setTagValue(info.overlayNode, m_tagKeys.vehicle, "scooter_rental");
                    break;
                case RentalVehicle::ElectricMoped:
                    OSM::setTagValue(info.overlayNode, m_tagKeys.vehicle, "motorcycle_rental");
                    break;
                case RentalVehicle::Car:
                    OSM::setTagValue(info.overlayNode, m_tagKeys.vehicle, "car_rental");
                    break;
            }
            OSM::setTagValue(info.overlayNode, m_tagKeys.name, loc.name().toUtf8());
            setTagIfMissing(info.overlayNode, m_tagKeys.network, vehicle.network().name());
            if (vehicle.remainingRange() >= 0) {
                OSM::setTagValue(info.overlayNode, m_tagKeys.remainingRange, QByteArray::number(vehicle.remainingRange()));
            }
            break;
        }
        case Location::Equipment:
            break;
    }
    return info;
}
