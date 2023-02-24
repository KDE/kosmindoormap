/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "platformmodel.h"
#include "platformfinder_p.h"

#include <QPointF>
#include <QRegularExpression>

#include <limits>

using namespace KOSMIndoorMap;

static constexpr auto TOP_PARENT = std::numeric_limits<quintptr>::max();

PlatformModel::PlatformModel(QObject* parent) :
    QAbstractItemModel(parent)
{
    m_matchTimer.setSingleShot(true);
    m_matchTimer.setInterval(0);
    connect(&m_matchTimer, &QTimer::timeout, this, &PlatformModel::matchPlatforms);

    connect(this, &PlatformModel::mapDataChanged, &m_matchTimer, qOverload<>(&QTimer::start));
    connect(this, &PlatformModel::arrivalPlatformChanged, &m_matchTimer, qOverload<>(&QTimer::start));
    connect(this, &PlatformModel::departurePlatformChanged, &m_matchTimer, qOverload<>(&QTimer::start));
}

PlatformModel::~PlatformModel() = default;

MapData PlatformModel::mapData() const
{
    return m_data;
}

void PlatformModel::setMapData(const MapData &data)
{
    if (m_data == data) {
        return;
    }

    beginResetModel();
    m_platforms.clear();
    m_platformLabels.clear();
    m_sectionsLabels.clear();
    m_arrivalPlatformRow = -1;
    m_departurePlatformRow = -1;

    m_data = data;
    if (!m_data.isEmpty()) {
        PlatformFinder finder;
        m_platforms = finder.find(m_data);

        m_tagKeys.arrival = m_data.dataSet().makeTagKey("mx:arrival");
        m_tagKeys.departure = m_data.dataSet().makeTagKey("mx:departure");
        createLabels();
    }
    endResetModel();
    Q_EMIT mapDataChanged();
    Q_EMIT platformIndexChanged();
}

bool PlatformModel::isEmpty() const
{
    return rowCount() == 0;
}

int PlatformModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 1;
}

int PlatformModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return parent.internalId() == TOP_PARENT ? m_platforms[parent.row()].sections().size() : 0;
    }

    return m_platforms.size();
}

QVariant PlatformModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }

    if (index.internalId() == TOP_PARENT) {
        const auto &platform = m_platforms[index.row()];
        switch (role) {
            case Qt::DisplayRole:
                return platform.name();
            case CoordinateRole:
                return QPointF(platform.position().lonF(), platform.position().latF());
            case ElementRole:
                return QVariant::fromValue(OSM::Element(m_platformLabels[index.row()]));
            case LevelRole:
                return platform.level();
            case TransportModeRole:
                return platform.mode();
            case LinesRole:
                return platform.lines();
            case ArrivalPlatformRole:
                return index.row() == m_arrivalPlatformRow;
            case DeparturePlatformRole:
                return index.row() == m_departurePlatformRow;
        }
    } else {
        const auto &platform = m_platforms[index.internalId()];
        const auto &section = platform.sections()[index.row()];
        switch (role) {
            case Qt::DisplayRole:
                return section.name();
            case CoordinateRole:
                return QPointF(section.position().center().lonF(), section.position().center().latF());
            case ElementRole:
                return QVariant::fromValue(OSM::Element(m_sectionsLabels[index.internalId()][index.row()]));
            case LevelRole:
                return platform.level();
        }
    }

    return {};
}

QModelIndex PlatformModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return createIndex(row, column, TOP_PARENT);
    }
    return createIndex(row, column, parent.row());
}

QModelIndex PlatformModel::parent(const QModelIndex &child) const
{
    if (!child.isValid() || child.internalId() == TOP_PARENT) {
        return {};
    }
    return createIndex(child.internalId(), 0, TOP_PARENT);
}

QHash<int, QByteArray> PlatformModel::roleNames() const
{
    auto n = QAbstractItemModel::roleNames();
    n.insert(CoordinateRole, "coordinate");
    n.insert(ElementRole, "osmElement");
    n.insert(LevelRole, "level");
    n.insert(TransportModeRole, "mode");
    n.insert(LinesRole, "lines");
    n.insert(ArrivalPlatformRole, "isArrivalPlatform");
    n.insert(DeparturePlatformRole, "isDeparturePlatform");
    return n;
}

Platform PlatformModel::arrivalPlatform() const
{
    return m_arrivalPlatform;
}

void PlatformModel::setArrivalPlatform(const Platform &platform)
{
    m_arrivalPlatform = platform;
    Q_EMIT arrivalPlatformChanged();
}

void PlatformModel::setArrivalPlatform(const QString &name, Platform::Mode mode)
{
    m_arrivalPlatform.setName(name);
    m_arrivalPlatform.setMode(mode);
    Q_EMIT arrivalPlatformChanged();
}

Platform PlatformModel::departurePlatform() const
{
    return m_departurePlatform;
}

void PlatformModel::setDeparturePlatform(const Platform &platform)
{
    m_departurePlatform = platform;
    Q_EMIT departurePlatformChanged();
}

void PlatformModel::setDeparturePlatform(const QString &name, Platform::Mode mode)
{
    m_departurePlatform.setName(name);
    m_departurePlatform.setMode(mode);
    Q_EMIT departurePlatformChanged();
}

int PlatformModel::arrivalPlatformRow() const
{
    return m_arrivalPlatformRow;
}

int PlatformModel::departurePlatformRow() const
{
    return m_departurePlatformRow;
}

void PlatformModel::matchPlatforms()
{
    setPlatformTag(m_arrivalPlatformRow, m_tagKeys.arrival, false);
    applySectionSelection(m_arrivalPlatformRow, m_tagKeys.arrival, {});
    m_arrivalPlatformRow = matchPlatform(m_arrivalPlatform);
    setPlatformTag(m_arrivalPlatformRow, m_tagKeys.arrival, true);
    setPlatformTag(m_departurePlatformRow, m_tagKeys.departure, false);
    applySectionSelection(m_departurePlatformRow, m_tagKeys.departure, {});
    m_departurePlatformRow = matchPlatform(m_departurePlatform);
    setPlatformTag(m_departurePlatformRow, m_tagKeys.departure, true);
    Q_EMIT platformIndexChanged();

    if (m_arrivalPlatformRow >= 0) {
        const auto idx = index(m_arrivalPlatformRow, 0);
        Q_EMIT dataChanged(idx, idx);
        applySectionSelection(m_arrivalPlatformRow, m_tagKeys.arrival, effectiveArrivalSections());
        Q_EMIT dataChanged(index(0, 0, idx), index(rowCount(idx) - 1, 0, idx));
    }
    if (m_departurePlatformRow >= 0) {
        const auto idx = index(m_departurePlatformRow, 0);
        Q_EMIT dataChanged(idx, idx);
        applySectionSelection(m_departurePlatformRow, m_tagKeys.departure, effectiveDepartureSections());
        Q_EMIT dataChanged(index(0, 0, idx), index(rowCount(idx) - 1, 0, idx));
    }
}

static bool isPossiblySamePlatformName(const QString &name, const QString &platform)
{
    // <platform>\w?<section(s)>
    if (name.size() > platform.size()) {
        QRegularExpression exp(QStringLiteral("(\\d+)\\s?[A-Z-]+"));
        const auto match = exp.match(name);
        return match.hasMatch() && match.captured(1) == platform;
    }

    return false;
}

int PlatformModel::matchPlatform(const Platform &platform) const
{
    if (!platform.ifopt().isEmpty()) { // try IFOPT first, if we have that
        const auto it = std::find_if(m_platforms.begin(), m_platforms.end(), [platform](const auto &p) {
            return p.ifopt() == platform.ifopt();
        });
        if (it != m_platforms.end()) {
            return std::distance(m_platforms.begin(), it);
        }
    }

    if (platform.name().isEmpty()) {
        return -1;
    }

    // exact match
    int i = 0;
    for (const auto &p : m_platforms) {
        if (p.name() == platform.name() && p.mode() == platform.mode()) {
            return i;
        }
        ++i;
    }

    // fuzzy match
    // TODO this likely will need to handle more scenarios
    // TODO when we get section ranges here, we might want to use those as well?
    i = 0;
    for (const auto &p : m_platforms) {
        if (p.mode() == platform.mode() && isPossiblySamePlatformName(platform.name(), p.name())) {
            return i;
        }
        ++i;
    }

    return -1;
}

void PlatformModel::createLabels()
{
    const auto platformTag = m_data.dataSet().makeTagKey("mx:platform");
    const auto sectionTag = m_data.dataSet().makeTagKey("mx:platform_section");

    m_platformLabels.reserve(m_platforms.size());
    m_sectionsLabels.resize(m_platforms.size());
    for (std::size_t i = 0; i < m_platforms.size(); ++i) {
        const auto &p = m_platforms[i];

        // TODO using the full edge/track path here might be better for layouting
        auto node = new OSM::Node;
        node->id = m_data.dataSet().nextInternalId();
        node->coordinate = p.position();
        OSM::setTagValue(*node, platformTag, p.name().toUtf8());
        m_platformLabels.push_back(OSM::UniqueElement(node));

        m_sectionsLabels[i].reserve(p.sections().size());
        for (const auto &sec : p.sections()) {
            auto node = new OSM::Node;
            node->id = m_data.dataSet().nextInternalId();
            node->coordinate = sec.position().center();
            OSM::setTagValue(*node, sectionTag, sec.name().toUtf8());
            m_sectionsLabels[i].push_back(OSM::UniqueElement(node));
        }
    }
}

void PlatformModel::setPlatformTag(int idx, OSM::TagKey key, bool enabled)
{
    if (idx < 0) {
        return;
    }

    m_platformLabels[idx].setTagValue(key, enabled ? "1" : "0");
}

static QStringView stripPlatform(QStringView p)
{
    while (!p.empty() && (p[0].isDigit() || p[0].isSpace())) {
        p = p.mid(1);
    }
    return p;
}

QStringView PlatformModel::effectiveArrivalSections() const
{
    // TODO prefer explicit section selectors once implemented/when present
    return stripPlatform(m_arrivalPlatform.name());
}

QStringView PlatformModel::effectiveDepartureSections() const
{
    // TODO prefer explicit section selectors once implemented/when present
    return stripPlatform(m_departurePlatform.name());
}

static std::vector<QChar> parseSectionSet(QStringView sections)
{
    std::vector<QChar> result;
    const auto ranges = sections.split(QLatin1Char(','));
    for (const auto &r : ranges) {
        if (r.size() == 1) {
            result.push_back(r[0]);
            continue;
        }
        if (r.size() == 3 && r[1] == QLatin1Char('-') && r[0] < r[2]) {
            for (QChar c = r[0]; c <= r[2]; c = QChar(c.unicode() + 1)) {
                result.push_back(c);
            }
            continue;
        }
        qDebug() << "failed to parse platform section expression:" << r;
    }
    return result;
}

void PlatformModel::applySectionSelection(int platformIdx, OSM::TagKey key, QStringView sections)
{
    if (platformIdx < 0) {
        return;
    }

    const auto sectionSet = parseSectionSet(sections);

    std::size_t totalSelected = 0;
    for (std::size_t i = 0; i < m_platforms[platformIdx].sections().size(); ++i) {
        if (std::any_of(sectionSet.begin(), sectionSet.end(), [this, i, platformIdx](const QChar s) {
            return s == m_platforms[platformIdx].sections()[i].name();
        })) {
            m_sectionsLabels[platformIdx][i].setTagValue(key, "1");
            ++totalSelected;
        } else {
            m_sectionsLabels[platformIdx][i].setTagValue(key, "0");
        }
    }

    // if we enabled all sections, disable them again, highlighting adds no value then
    if (totalSelected == m_sectionsLabels[platformIdx].size()) {
        for (auto &s : m_sectionsLabels[platformIdx]) {
            s.setTagValue(key, "0");
        }
    }
}

#include "moc_platformmodel.cpp"
