/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORMAP_AMENITYMODEL_H
#define KOSMINDOORMAP_AMENITYMODEL_H

#include <KOSMIndoorMap/MapData>
#include <KOSMIndoorMap/MapCSSStyle>

#include <KOSM/Element>

#include <QAbstractListModel>

namespace KOSMIndoorMap {

/** List all amenities in a given data set. */
class AmenityModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(KOSMIndoorMap::MapData mapData READ mapData WRITE setMapData NOTIFY mapDataChanged)
public:
    explicit AmenityModel(QObject *parent = nullptr);
    ~AmenityModel();

    [[nodiscard]] MapData mapData() const;
    void setMapData(const MapData &data);

    enum Role {
        NameRole = Qt::DisplayRole,
        CoordinateRole = Qt::UserRole,
        LevelRole,
        ElementRole,
        TypeNameRole,
        GroupRole,
        GroupNameRole,
        IconSourceRole,
        CuisineRole, ///< details on entries in the FoodGroup
        FallbackNameRole, ///< Brand/operator/network name, better than nothing but not the first choice to display
        OpeningHoursRole, ///< opening hours expression
    };
    Q_ENUM(Role)

    enum Group {
        UndefinedGroup,
        FoodGroup,
        ShopGroup,
        ToiletGroup,
        AmenityGroup,
        HealthcareGroup,
        AccommodationGroup,
    };
    Q_ENUM(Group)

    [[nodiscard]] int rowCount(const QModelIndex &parent = {}) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

Q_SIGNALS:
    void mapDataChanged();

private:
    struct Entry {
        OSM::Element element;
        int level;
        Group group = UndefinedGroup;
        QByteArray typeKey;
        QString icon;
    };

    void populateModel();
    static QString iconSource(const Entry &entry);

    MapData m_data;
    MapCSSStyle m_style;
    std::vector<Entry> m_entries;
};

}

#endif // KOSMINDOORMAP_AMENITYMODEL_H
