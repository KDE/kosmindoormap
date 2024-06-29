/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "amenitymodel.h"
#include "localization.h"
#include "logging.h"
#include "osmelement.h"

#include <style/mapcssdeclaration_p.h>
#include <style/mapcssstate_p.h>

#include <KOSMIndoorMap/MapCSSParser>
#include <KOSMIndoorMap/MapCSSResult>

#include <KCountry>
#include <KCountrySubdivision>
#include <KLocalizedString>

#include <QDebug>
#include <QFile>
#include <QPointF>
#include <QTimeZone>

#include <limits>

using namespace KOSMIndoorMap;

AmenityModel::AmenityModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_langs(OSM::Languages::fromQLocale(QLocale()))
{
}

AmenityModel::~AmenityModel() = default;

MapData AmenityModel::mapData() const
{
    return m_data;
}

void AmenityModel::setMapData(const MapData &data)
{
    if (m_data == data) {
        return;
    }

    if (m_style.isEmpty()) {
        MapCSSParser p;
        m_style = p.parse(QStringLiteral(":/org.kde.kosmindoormap/assets/quick/amenity-model.mapcss"));
        if (p.hasError()) {
            qWarning() << p.errorMessage();
            return;
        }
    }

    beginResetModel();
    m_entries.clear();
    m_data = data;
    if (!m_data.isEmpty()) {
        m_style.compile(m_data.dataSet());
    }
    endResetModel();
    Q_EMIT mapDataChanged();
}

int AmenityModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    if (m_entries.empty() && !m_data.isEmpty()) {
        // we assume that this is expensive but almost never will result in an empty result
        // and if it does nevertheless, it's a sparsely populated tile where this is cheap
        const_cast<AmenityModel*>(this)->populateModel();
    }

    return (int)m_entries.size();
}

static QString groupName(AmenityModel::Group group)
{
    switch (group) {
        case AmenityModel::UndefinedGroup:
            return {};
        case AmenityModel::FoodGroup:
            return i18nc("amenity category", "Food & Drinks");
        case AmenityModel::ShopGroup:
            return i18nc("amenity category", "Shops");
        case AmenityModel::ToiletGroup:
            return i18nc("amenity category", "Toilets");
        case AmenityModel::HealthcareGroup:
            return i18nc("amenity category", "Healthcare");
        case AmenityModel::AmenityGroup:
            return i18nc("amenity category", "Amenities");
        case AmenityModel::AccommodationGroup:
            return i18nc("amenity category", "Accommodations");
    }
    return {};
}

QString AmenityModel::iconSource(const AmenityModel::Entry &entry)
{
    QString s = QLatin1String(":/org.kde.kosmindoormap/assets/icons/") + entry.icon + QLatin1String(".svg");
    return QFile::exists(s) ? s : QStringLiteral("map-symbolic");
}

QVariant AmenityModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index)) {
        return {};
    }

    const auto &entry = m_entries[index.row()];
    switch (role) {
        case Qt::DisplayRole:
            return QString::fromUtf8(entry.element.tagValue(m_langs, "name", "loc_name", "int_name"));
            // TODO see name transliteration in OSM info model
        case TypeNameRole:
            return Localization::amenityTypes(entry.element.tagValue(entry.typeKey.constData()), Localization::ReturnEmptyOnUnknownKey);
        case CoordinateRole:
        {
            const auto center = entry.element.center();
            return QPointF(center.lonF(), center.latF());
        }
        case LevelRole:
            return entry.level;
        case ElementRole:
            return QVariant::fromValue(OSMElement(entry.element));
        case GroupRole:
            return entry.group;
        case GroupNameRole:
            return groupName(entry.group);
        case IconSourceRole:
            return iconSource(entry);
        case CuisineRole:
        {
            auto s = Localization::cuisineTypes(entry.element.tagValue("cuisine"), Localization::ReturnEmptyOnUnknownKey);
            if (!s.isEmpty()) {
                return s;
            }
            return Localization::amenityTypes(entry.element.tagValue("vending"), Localization::ReturnEmptyOnUnknownKey);
        }
        case FallbackNameRole:
            return QString::fromUtf8(entry.element.tagValue(m_langs, "brand", "operator", "network"));
        case OpeningHoursRole:
            return QString::fromUtf8(entry.element.tagValue("opening_hours"));
        case TimeZoneRole:
            return QString::fromUtf8(m_data.timeZone().id());
        case RegionCodeRole:
            if (m_data.regionCode().size() > 3) {
                return m_data.regionCode();
            }
            if (const auto subdiv = KCountrySubdivision::fromLocation((float)entry.element.center().latF(), (float)entry.element.center().lonF()); subdiv.isValid()) {
                return subdiv.code();
            }
            if (const auto c = KCountry::fromLocation((float)entry.element.center().latF(), (float)entry.element.center().lonF()); c.isValid()) {
                return c.alpha2();
            }
            return m_data.regionCode();
    }

    return {};
}

QHash<int, QByteArray> AmenityModel::roleNames() const
{
    auto r = QAbstractListModel::roleNames();
    r.insert(NameRole, "name");
    r.insert(TypeNameRole, "typeName");
    r.insert(CoordinateRole, "coordinate");
    r.insert(LevelRole, "level");
    r.insert(ElementRole, "element");
    r.insert(GroupRole, "group");
    r.insert(GroupNameRole, "groupName");
    r.insert(IconSourceRole, "iconSource");
    r.insert(CuisineRole, "cuisine");
    r.insert(FallbackNameRole, "fallbackName");
    r.insert(OpeningHoursRole, "openingHours");
    r.insert(TimeZoneRole, "timeZone");
    r.insert(RegionCodeRole, "regionCode");
    return r;
}

struct {
    const char *groupName;
    AmenityModel::Group group;
} constexpr const group_map[] = {
    { "accommodation", AmenityModel::AccommodationGroup },
    { "amenity", AmenityModel::AmenityGroup },
    { "healthcare", AmenityModel::HealthcareGroup },
    { "food", AmenityModel::FoodGroup },
    { "shop", AmenityModel::ShopGroup },
    { "toilets", AmenityModel::ToiletGroup },
};

void AmenityModel::populateModel()
{
    const auto layerKey = m_data.dataSet().tagKey("layer");

    MapCSSResult filterResult;
    for (auto it = m_data.levelMap().begin(); it != m_data.levelMap().end(); ++it) {
        for (const auto &e : (*it).second) {
            if (!OSM::contains(m_data.boundingBox(), e.center())) {
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

            const auto group = res.tagValue(layerKey);
            const auto groupIt = std::find_if(std::begin(group_map), std::end(group_map), [&group](const auto &m) { return std::strcmp(m.groupName, group.constData()) == 0; });
            if (groupIt == std::end(group_map)) {
                continue; // no group assigned
            }

            Entry entry;
            entry.element = e;
            entry.group = (*groupIt).group;

            QByteArray typeKey;
            if (auto prop = res.declaration(MapCSSProperty::FontFamily); prop) {
                typeKey = prop->keyValue();
            }
            if (typeKey.isEmpty()) {
                continue;
            }

            const auto types = e.tagValue(typeKey.constData()).split(';');
            for (const auto &type : types) {
                if (Localization::hasAmenityTypeTranslation(type.trimmed().constData())) {
                    entry.typeKey = std::move(typeKey);
                    break;
                }
            }
            if (entry.typeKey.isEmpty()) {
                qCDebug(Log) << "unknown type: " << types << e.url();
                continue;
            }

            if (auto prop = res.declaration(MapCSSProperty::IconImage); prop) {
                entry.icon = prop->stringValue();
                if (entry.icon.isEmpty()) {
                    entry.icon = QString::fromUtf8(e.tagValue(prop->keyValue().constData()));
                }
            }

            entry.level = (*it).first.numericLevel(); // TODO we only need one entry, not one per level!
            m_entries.push_back(std::move(entry));
        }
    }

    // de-duplicate multi-level entries
    // we could also just iterate over the non-level-split data, but
    // then we need to reparse the level data here...
    std::sort(m_entries.begin(), m_entries.end(), [](const auto &lhs, const auto &rhs) {
        if (lhs.element == rhs.element) {
            return std::abs(lhs.level) < std::abs(rhs.level);
        }
        return lhs.element < rhs.element;
    });
    m_entries.erase(std::unique(m_entries.begin(), m_entries.end(), [](const auto &lhs, const auto &rhs) {
        return lhs.element == rhs.element;
    }), m_entries.end());

    // sort by group
    std::sort(m_entries.begin(), m_entries.end(), [](const auto &lhs, const auto &rhs) {
        return lhs.group < rhs.group;
    });
    qCDebug(Log) << m_entries.size() << "amenities found";
}

#include "moc_amenitymodel.cpp"
