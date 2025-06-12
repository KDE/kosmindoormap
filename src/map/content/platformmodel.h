/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORMAP_PLATFORMMODEL_H
#define KOSMINDOORMAP_PLATFORMMODEL_H

#include "kosmindoormap_export.h"

#include <KOSMIndoorMap/MapData>
#include <KOSMIndoorMap/Platform>

#include <QAbstractItemModel>
#include <QTimer>

namespace KOSMIndoorMap {

/** Lists all platforms/tracks and platform sections found in the current map.
 *  There's also the concept of (optional) arrival/departure platforms in here to highlight
 *  arriving/departing locations when used in context of a planned journey.
 */
class KOSMINDOORMAP_EXPORT PlatformModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_PROPERTY(KOSMIndoorMap::MapData mapData READ mapData WRITE setMapData NOTIFY mapDataChanged)
    Q_PROPERTY(bool isEmpty READ isEmpty NOTIFY mapDataChanged)

    /** Row indexes of the matched arrival/departure platforms, if found and/or set, otherwise @c -1. */
    Q_PROPERTY(int arrivalPlatformRow READ arrivalPlatformRow NOTIFY platformIndexChanged)
    Q_PROPERTY(int departurePlatformRow READ departurePlatformRow NOTIFY platformIndexChanged)

    /** Platform search parameters (name/mode/ifopt) for matching arrival/departure platform against what we found in the map data. */
    Q_PROPERTY(KOSMIndoorMap::Platform arrivalPlatform READ arrivalPlatform WRITE setArrivalPlatform NOTIFY arrivalPlatformChanged)
    Q_PROPERTY(KOSMIndoorMap::Platform departurePlatform READ departurePlatform WRITE setDeparturePlatform NOTIFY departurePlatformChanged)

public:
    explicit PlatformModel(QObject *parent = nullptr);
    ~PlatformModel();

    [[nodiscard]] MapData mapData() const;
    void setMapData(const MapData &data);

    [[nodiscard]] bool isEmpty() const;

    enum Role {
        CoordinateRole = Qt::UserRole,
        ElementRole,
        LevelRole,
        TransportModeRole,
        LinesRole,
        ArrivalPlatformRole,
        DeparturePlatformRole,
    };
    Q_ENUM(Role)

    [[nodiscard]] int columnCount(const QModelIndex &parent = {}) const override;
    [[nodiscard]] int rowCount(const QModelIndex &parent = {}) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
    [[nodiscard]] QModelIndex index(int row, int column, const QModelIndex &parent = {}) const override;
    [[nodiscard]] QModelIndex parent(const QModelIndex &child) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    /** Match arrival/departure platform against what we found in the map data. */
    [[nodiscard]] Platform arrivalPlatform() const;
    void setArrivalPlatform(const Platform &platform);
    Q_INVOKABLE [[deprecated("use arrivalPlatform property")]] void setArrivalPlatform(const QString &name, KOSMIndoorMap::Platform::Mode mode);
    [[nodiscard]] Platform departurePlatform() const;
    void setDeparturePlatform(const Platform &platform);
    Q_INVOKABLE [[deprecated("use departurePlatform property")]] void setDeparturePlatform(const QString &name, KOSMIndoorMap::Platform::Mode mode);

    [[nodiscard]] int arrivalPlatformRow() const;
    [[nodiscard]] int departurePlatformRow() const;

Q_SIGNALS:
    void mapDataChanged();
    void platformIndexChanged();
    void arrivalPlatformChanged();
    void departurePlatformChanged();

private:
    void matchPlatforms();
    [[nodiscard]] int matchPlatform(const Platform &platform) const;
    void createLabels();
    void setPlatformTag(int idx, OSM::TagKey key, bool enabled);

    [[nodiscard]] QStringView effectiveArrivalSections() const;
    [[nodiscard]] QStringView effectiveDepartureSections() const;
    void applySectionSelection(int platformIdx, OSM::TagKey key, QStringView sections);

    std::vector<Platform> m_platforms;
    MapData m_data;
    struct {
        OSM::TagKey arrival;
        OSM::TagKey departure;
    } m_tagKeys;

    std::vector<OSM::UniqueElement> m_platformLabels;
    std::vector<std::vector<OSM::UniqueElement>> m_sectionsLabels;

    Platform m_arrivalPlatform;
    Platform m_departurePlatform;
    int m_arrivalPlatformRow = -1;
    int m_departurePlatformRow = -1;

    QTimer m_matchTimer;
};

}

#endif // KOSMINDOORMAP_PLATFORMMODEL_H
