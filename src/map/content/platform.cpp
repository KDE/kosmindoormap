/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "platform.h"

#include <osm/element.h>
#include <osm/geomath.h>
#include <osm/pathutil.h>

#include <QRegularExpression>

#include <limits>

using namespace KOSMIndoorMap;

namespace KOSMIndoorMap {
class PlatformSectionPrivate : public QSharedData
{
public:
    QString name;
    OSM::Element position;
};
}

PlatformSection::PlatformSection()
    : d(new PlatformSectionPrivate)
{
}

PlatformSection::PlatformSection(const PlatformSection&) = default;
PlatformSection::PlatformSection(PlatformSection&&) = default;
PlatformSection::~PlatformSection() = default;
PlatformSection& PlatformSection::operator=(const PlatformSection&) = default;
PlatformSection& PlatformSection::operator=(PlatformSection&&) = default;

QString PlatformSection::name() const
{
    return d->name;
}

void PlatformSection::setName(const QString &name)
{
    d.detach();
    d->name = name;
}

OSM::Element PlatformSection::position() const
{
    return d->position;
}

void PlatformSection::setPosition(const OSM::Element &position)
{
    d.detach();
    d->position = position;
}

bool PlatformSection::isValid() const
{
    return !d->name.isEmpty() && d->position;
}


namespace KOSMIndoorMap {
class PlatformPrivate : public QSharedData
{
public:
    QString m_name;
    OSM::Element m_stopPoint;
    OSM::Element m_edge;
    OSM::Element m_area;
    std::vector<OSM::Element> m_track;
    Platform::Mode m_mode = Platform::Rail; // TODO should eventually be "Unknown"
    int m_level = std::numeric_limits<int>::min(); // INT_MIN indicates not set, needed for merging
    std::vector<PlatformSection> m_sections;
    QString m_ifopt;
    QStringList m_lines;

    static void appendSection(std::vector<PlatformSection> &sections, const Platform &p, PlatformSection &&sec, std::vector<const OSM::Node*> &edgePath, const OSM::DataSet &dataSet);
    static double maxSectionDistance(const Platform &p, const std::vector<PlatformSection> &sections, const OSM::DataSet &dataSet);
};
}

Platform::Platform()
    : d(new PlatformPrivate)
{
}

Platform::Platform(const Platform&) = default;
Platform::Platform(Platform&&) = default;
Platform::~Platform() = default;
Platform& Platform::operator=(const Platform&) = default;
Platform& Platform::operator=(Platform&&) = default;

bool Platform::isValid() const
{
    return !d->m_name.isEmpty() && position().isValid() && d->m_mode != Unknown;
}

QString Platform::name() const
{
    return d->m_name;
}

void Platform::setName(const QString &name)
{
    d.detach();
    d->m_name = name;
}

int Platform::level() const
{
    return hasLevel() ? d->m_level : 0;
}

bool Platform::hasLevel() const
{
    return d->m_level != std::numeric_limits<int>::min();
}

void Platform::setLevel(int level)
{
    d.detach();
    d->m_level = level;
}

OSM::Coordinate Platform::position() const
{
    return OSM::coalesce(d->m_stopPoint, d->m_area).center();
}

OSM::Element Platform::stopPoint() const
{
    return d->m_stopPoint;
}

void Platform::setStopPoint(OSM::Element stop)
{
    d.detach();
    d->m_stopPoint = stop;
}

OSM::Element Platform::edge() const
{
    return OSM::coalesce(d->m_edge, d->m_stopPoint);
}

void Platform::setEdge(OSM::Element edge)
{
    d.detach();
    d->m_edge = edge;
}

OSM::Element Platform::area() const
{
    return OSM::coalesce(d->m_area, d->m_edge, d->m_stopPoint);
}

void Platform::setArea(OSM::Element area)
{
    d.detach();
    d->m_area = area;
}

const std::vector<OSM::Element>& Platform::track() const
{
    return d->m_track;
}

void Platform::setTrack(std::vector<OSM::Element> &&track)
{
    d.detach();
    d->m_track = std::move(track);
}

std::vector<OSM::Element>&& Platform::takeTrack()
{
    d.detach();
    return std::move(d->m_track);
}

const std::vector<PlatformSection>& Platform::sections() const
{
    return d->m_sections;
}

void Platform::setSections(std::vector<PlatformSection> &&sections)
{
    d.detach();
    d->m_sections = std::move(sections);
}

std::vector<PlatformSection>&& Platform::takeSections()
{
    d.detach();
    return std::move(d->m_sections);
}

Platform::Mode Platform::mode() const
{
    return d->m_mode;
}

void Platform::setMode(Platform::Mode mode)
{
    d.detach();
    d->m_mode = mode;
}

QString Platform::ifopt() const
{
    return d->m_ifopt;
}

void Platform::setIfopt(const QString &ifopt)
{
    d.detach();
    d->m_ifopt = ifopt;
}

QStringList Platform::lines() const
{
    return d->m_lines;
}

void Platform::setLines(QStringList &&lines)
{
    d.detach();
    d->m_lines = std::move(lines);
}

QStringList&& Platform::takeLines()
{
    d.detach();
    return std::move(d->m_lines);
}

static bool conflictIfPresent(OSM::Element lhs, OSM::Element rhs)
{
    return lhs && rhs && lhs != rhs;
}

static bool equalIfPresent(OSM::Element lhs, OSM::Element rhs)
{
    return lhs && rhs && lhs == rhs;
}

static bool isSubPath(const std::vector<const OSM::Node*> &path, const OSM::Way &way)
{
    return std::all_of(way.nodes.begin(), way.nodes.end(), [&path](OSM::Id node) {
        return std::find_if(path.begin(), path.end(), [node](auto n) { return n->id == node; }) != path.end();
    });
}

static constexpr const auto MAX_TRACK_TO_EDGE_DISTANCE = 4.5; // meters
static constexpr const auto MAX_SECTION_TO_EDGE_DISTANCE = 5.0;

static double maxSectionDistance(const std::vector<const OSM::Node*> &path, const std::vector<PlatformSection> &sections)
{
    auto dist = std::numeric_limits<double>::lowest();
    for (const auto &section : sections) {
        dist = std::max(dist, OSM::distance(path, section.position().center()));
    }
    return dist;
}

double PlatformPrivate::maxSectionDistance(const Platform &p, const std::vector<PlatformSection> &sections, const OSM::DataSet &dataSet)
{
    if (auto elem = OSM::coalesce(p.d->m_edge, p.d->m_area)) {
        return ::maxSectionDistance(elem.outerPath(dataSet), sections);
    }
    if (!p.d->m_track.empty()) {
        std::vector<const OSM::Node*> path;
        OSM::assemblePath(dataSet, p.d->m_track, path);
        return ::maxSectionDistance(path, sections);
    }
    return std::numeric_limits<double>::lowest();
}

static const OSM::Way* outerWay(OSM::Element &multiPolygon, const OSM::DataSet &dataSet)
{
    // ### this assumes multi-polygons are structured in the way the Marble generator normalizes them!
    for (const auto &mem : multiPolygon.relation()->members) {
        if (mem.type() == OSM::Type::Way && (std::strcmp(mem.role().name(), "outer") == 0)) {
            return dataSet.way(mem.id);
        }
    }
    return nullptr;
}

static bool isConnectedGeometry(OSM::Element lhs, OSM::Element rhs, const OSM::DataSet &dataSet)
{
    if (lhs == rhs) { return false; }
    const OSM::Way *lway = nullptr;
    const OSM::Way *rway = nullptr;

    switch (lhs.type()) {
        case OSM::Type::Null:
        case OSM::Type::Node:
            return false;
        case OSM::Type::Way:
            lway = lhs.way();
            break;
        case OSM::Type::Relation:
            lway  = outerWay(lhs, dataSet);
            break;

    }
    switch (rhs.type()) {
        case OSM::Type::Null:
        case OSM::Type::Node:
            return false;
        case OSM::Type::Way:
            rway = rhs.way();
            break;
        case OSM::Type::Relation:
            rway = outerWay(rhs, dataSet);
            break;
    }
    if (!lway || !rway) {
        return false;
    }

    if (!lway->isClosed() && !rway->isClosed()) {
        return lway->nodes.front() == rway->nodes.front()
            || lway->nodes.back() == rway->nodes.front()
            || lway->nodes.front() == rway->nodes.back()
            || lway->nodes.back() == rway->nodes.back();
    }
    if (lway->isClosed() && lway->nodes.size() > 2 && rway->isClosed() && rway->nodes.size() > 2) {
        // ### this assumes multi-polygons are structured in the way the Marble generator normalizes them!
        bool found = false;
        for (std::size_t i = 0; i < lway->nodes.size() && !found; ++i) {
            const auto n1 = lway->nodes[i];
            const auto n2 = lway->nodes[(i + 1) % lway->nodes.size()];
            for (std::size_t j = 0; j < rway->nodes.size() && !found; ++j) {
                found = ((n1 == rway->nodes[j]) && (n2 == rway->nodes[(j + 1) % rway->nodes.size()]))
                     || ((n2 == rway->nodes[j]) && (n1 == rway->nodes[(j + 1) % rway->nodes.size()]));
            }
        }
        return found;
    }

    return false;
}

static bool isConnectedWay(const std::vector<OSM::Element> &lhs, const std::vector<OSM::Element> &rhs, const OSM::DataSet &dataSet)
{
    return std::any_of(lhs.begin(), lhs.end(), [&](auto lway) {
        return std::any_of(rhs.begin(), rhs.end(), [&](auto rway) {
            return isConnectedGeometry(lway, rway, dataSet);
        });
    });
}

static bool isOverlappingWay(const std::vector<OSM::Element> &lhs, const std::vector<OSM::Element> &rhs)
{
    return std::any_of(lhs.begin(), lhs.end(), [&](auto lway) {
        return std::any_of(rhs.begin(), rhs.end(), [&](auto rway) {
            return lway == rway;
        });
    });
}

bool Platform::isSame(const Platform &lhs, const Platform &rhs, const OSM::DataSet &dataSet)
{
    if (!lhs.ifopt().isEmpty() && !rhs.ifopt().isEmpty()) {
        return lhs.ifopt() == rhs.ifopt();
    }

    const auto isConnectedEdge = isConnectedGeometry(lhs.d->m_edge, rhs.d->m_edge, dataSet);
    const auto isConnectedTrack = isConnectedWay(lhs.d->m_track, rhs.d->m_track, dataSet);
    const auto isOverlappingTrack = isOverlappingWay(lhs.d->m_track, rhs.d->m_track);
    const auto isConnectedArea = isConnectedGeometry(lhs.d->m_area, rhs.d->m_area, dataSet);

    if ((conflictIfPresent(lhs.d->m_stopPoint, rhs.d->m_stopPoint) && lhs.d->m_track != rhs.d->m_track && !isConnectedTrack)
     || (conflictIfPresent(lhs.d->m_edge, rhs.d->m_edge) && !isConnectedEdge)
     || (conflictIfPresent(lhs.d->m_area, rhs.d->m_area) && !isConnectedArea)
     || (!lhs.d->m_track.empty() && !rhs.d->m_track.empty() && !isOverlappingTrack && !isConnectedTrack)
     || (lhs.hasLevel() && rhs.hasLevel() && lhs.level() != rhs.level())
     || (lhs.d->m_mode != Unknown && rhs.d->m_mode != Unknown && lhs.d->m_mode != rhs.d->m_mode))
    {
        return false;
    }

    // we can accept conflicting names if one of them is likely a station name instead of a platform name
    if (!lhs.d->m_name.isEmpty() && !rhs.d->m_name.isEmpty() && lhs.d->m_name != rhs.d->m_name) {
        if (isPlausibleName(lhs.name()) && isPlausibleName(rhs.name())) {
            return false;
        }
    }

    // edge has to be part of area, but on its own that doesn't mean equallity
    if (!isConnectedArea && !isConnectedEdge) {
        if ((lhs.d->m_area && rhs.d->m_edge.type() == OSM::Type::Way && !isSubPath(lhs.d->m_area.outerPath(dataSet), *rhs.d->m_edge.way()))
        || (rhs.d->m_area && lhs.d->m_edge.type() == OSM::Type::Way && !isSubPath(rhs.d->m_area.outerPath(dataSet), *lhs.d->m_edge.way()))) {
            return false;
        }
    }

    // matching edge, point or track is good enough, matching area however isn't
    if (equalIfPresent(lhs.d->m_stopPoint, rhs.d->m_stopPoint)
     || equalIfPresent(lhs.d->m_edge, rhs.d->m_edge) || isConnectedEdge
     || isOverlappingTrack)
    {
        return true;
    }

    if (!isConnectedEdge) {
        // track/stop and area/edge elements do not share nodes, so those we need to match by spatial distance
        if (lhs.d->m_edge && rhs.d->m_stopPoint) {
            return OSM::distance(lhs.d->m_edge.outerPath(dataSet), rhs.position()) < MAX_TRACK_TO_EDGE_DISTANCE;
        }
        if (rhs.d->m_edge && lhs.d->m_stopPoint) {
            return OSM::distance(rhs.d->m_edge.outerPath(dataSet), lhs.position()) < MAX_TRACK_TO_EDGE_DISTANCE;
        }
    }

    if (!isConnectedArea) {
        if (lhs.d->m_area && rhs.d->m_stopPoint) {
            return OSM::distance(lhs.d->m_area.outerPath(dataSet), rhs.position()) < MAX_TRACK_TO_EDGE_DISTANCE;
        }
        if (rhs.d->m_area && lhs.d->m_stopPoint) {
            return OSM::distance(rhs.d->m_area.outerPath(dataSet), lhs.position()) < MAX_TRACK_TO_EDGE_DISTANCE;
        }
    }

    // free-floating sections: edge, area or track is within a reasonable distance
    if (((lhs.d->m_name.isEmpty() ^ rhs.d->m_name.isEmpty()) || lhs.d->m_name == rhs.d->m_name) && !isConnectedArea && !isConnectedEdge) {
        auto d = PlatformPrivate::maxSectionDistance(lhs, rhs.sections(), dataSet);
        if (d >= 0.0) {
            return d < MAX_SECTION_TO_EDGE_DISTANCE;
        }
        d = PlatformPrivate::maxSectionDistance(rhs, lhs.sections(), dataSet);
        if (d >= 0.0) {
            return d < MAX_SECTION_TO_EDGE_DISTANCE;
        }
    }

    return isConnectedArea || isConnectedEdge || isConnectedTrack;
}

static bool compareSection(const PlatformSection &lhs, const PlatformSection &rhs)
{
    if (lhs.name() == rhs.name()) {
        return lhs.position() < rhs.position();
    }
    return lhs.name() < rhs.name();
}

void PlatformPrivate::appendSection(std::vector<PlatformSection> &sections, const Platform &p, PlatformSection &&sec, std::vector<const OSM::Node*> &edgePath, const OSM::DataSet &dataSet)
{
    if (sections.empty() || sections.back().name() != sec.name()) {
        sections.push_back(std::move(sec));
        return;
    }

    // check which one is closer
    if (edgePath.empty()) {
        if (p.d->m_edge) {
            edgePath = p.d->m_edge.outerPath(dataSet);
        } else if (!p.d->m_track.empty()) {
            OSM::assemblePath(dataSet, p.d->m_track, edgePath);
        }
    }
    const auto dist1 = OSM::distance(edgePath, sections.back().position().center());
    const auto dist2 = OSM::distance(edgePath, sec.position().center());
    if (dist2 < dist1) {
        sections.back() = std::move(sec);
    }
}

static std::vector<OSM::Element> mergeWays(const std::vector<OSM::Element> &lhs, const std::vector<OSM::Element> &rhs)
{
    std::vector<OSM::Element> w = lhs;
    for (auto e : rhs) {
        if (std::find(w.begin(), w.end(), e) == w.end()) {
            w.push_back(e);
        }
    }
    return w;
}

Platform Platform::merge(const Platform &lhs, const Platform &rhs, const OSM::DataSet &dataSet)
{
    Platform p;
    p.d->m_name = preferredName(lhs.name(), rhs.name());
    p.d->m_stopPoint = OSM::coalesce(lhs.d->m_stopPoint, rhs.d->m_stopPoint);
    p.d->m_edge = OSM::coalesce(lhs.d->m_edge, rhs.d->m_edge);
    p.d->m_area = OSM::coalesce(lhs.d->m_area, rhs.d->m_area);
    p.d->m_track = mergeWays(lhs.d->m_track, rhs.d->m_track);
    p.d->m_level = lhs.hasLevel() ? lhs.d->m_level : rhs.d->m_level;
    p.d->m_ifopt = lhs.ifopt().isEmpty() ? rhs.ifopt() : lhs.ifopt();

    // TODO
    p.d->m_mode = std::max(lhs.d->m_mode, rhs.d->m_mode);
    p.d->m_lines = lhs.d->m_lines.isEmpty() ? std::move(rhs.d->m_lines) : std::move(lhs.d->m_lines);

    std::vector<const OSM::Node*> edgePath;
    std::vector<PlatformSection> sections;
    auto lsec = lhs.sections();
    auto rsec = rhs.sections();
    std::sort(lsec.begin(), lsec.end(), compareSection);
    std::sort(rsec.begin(), rsec.end(), compareSection);
    for (auto lit = lsec.begin(), rit = rsec.begin(); lit != lsec.end() || rit != rsec.end();) {
        if (rit == rsec.end()) {
            PlatformPrivate::appendSection(sections, p, std::move(*lit++), edgePath, dataSet);
            continue;
        }
        if (lit == lsec.end()) {
            PlatformPrivate::appendSection(sections, p, std::move(*rit++), edgePath, dataSet);
            continue;
        }
        if (compareSection(*lit, *rit)) {
            PlatformPrivate::appendSection(sections, p, std::move(*lit++), edgePath, dataSet);
            continue;
        }
        if (compareSection(*rit, *lit)) {
            PlatformPrivate::appendSection(sections, p, std::move(*rit++), edgePath, dataSet);
            continue;
        }

        // both are equal
        if ((*lit).position() == (*rit).position()) {
            PlatformPrivate::appendSection(sections, p, std::move(*lit++), edgePath, dataSet);
            ++rit;
            continue;
        }

        // both are equal but differ in distance: will be handled in appendSection in the next iteration
        PlatformPrivate::appendSection(sections, p, std::move(*lit++), edgePath, dataSet);
    }
    p.setSections(std::move(sections));

    return p;
}

bool Platform::isPlausibleName(const QString &name)
{
    static QRegularExpression exp(QStringLiteral("^(\\d{1,3}[a-z]?|[A-Z])$"));
    return exp.match(name).hasMatch();
}

QString Platform::preferredName(const QString &lhs, const QString &rhs)
{
    if (lhs.isEmpty()) {
        return rhs;
    }
    if (rhs.isEmpty()) {
        return lhs;
    }

    if (isPlausibleName(lhs)) {
        return lhs;
    }
    if (isPlausibleName(rhs)) {
        return rhs;
    }

    return lhs.size() <= rhs.size() ? lhs: rhs;
}
