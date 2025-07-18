/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "platformfinder_p.h"

#include <QRegularExpression>

using namespace KOSMIndoorMap;

PlatformFinder::PlatformFinder()
{
    m_collator.setLocale(QLocale());
    m_collator.setNumericMode(true);
    m_collator.setIgnorePunctuation(true);
    m_collator.setCaseSensitivity(Qt::CaseInsensitive);
}

PlatformFinder::~PlatformFinder() = default;

static QString nameFromTrack(OSM::Element track)
{
    auto name = QString::fromUtf8(track.tagValue("railway:track_ref"));
    if (!name.isEmpty()) {
        return name;
    }

    name = QString::fromUtf8(track.tagValue("name"));
    for (const char *n : { "platform", "voie", "gleis" }) {
        if (name.contains(QLatin1String(n), Qt::CaseInsensitive)) {
            return name.remove(QLatin1String(n), Qt::CaseInsensitive).trimmed();
        }
    }

    return {};
}

std::vector<Platform> PlatformFinder::find(const MapData &data)
{
    m_data = data;
    resolveTagKeys();

    for (auto it = m_data.levelMap().begin(); it != m_data.levelMap().end(); ++it) {
        for (const auto &e : (*it).second) {
            if (!e.hasTags() || e.tagValue(m_tagKeys.disused) == "yes") {
                continue;
            }

            // free floating section signs
            const auto elemPt = e.tagValue(m_tagKeys.public_transport);
            if (elemPt == "platform_section_sign") {
                const auto platformRef = e.tagValue(m_tagKeys.platform_ref);
                const auto ref = e.tagValue("ref");
                if (!platformRef.isEmpty() && !ref.isEmpty()) {
                    Platform p;
                    p.setLevel(levelForPlatform((*it).first, e));
                    p.setName(QString::fromUtf8(platformRef));
                    PlatformSection section;
                    section.setName(QString::fromUtf8(ref));
                    section.setPosition(e);
                    p.setSections({section});
                    m_floatingSections.push_back(std::move(p));
                }
            }

            // non-standard free-floating section signs
            if (const auto platformRef = e.tagValue(m_tagKeys.platform_colon_ref); !platformRef.isEmpty() && e.tagValue("pole") == "landmark_sign") {
                const auto names = QString::fromUtf8(platformRef).split(QLatin1Char(';'));
                for (const auto &name : names) {
                    Platform p;
                    p.setLevel(levelForPlatform((*it).first, e));
                    p.setName(name);
                    PlatformSection section;
                    section.setName(QString::fromUtf8(e.tagValue("local_ref", "ref")));
                    section.setPosition(e);
                    p.setSections({section});
                    m_floatingSections.push_back(std::move(p)); // can't merge this reliably until we have the full area geometry
                }
            }
            if (e.tagValue(m_tagKeys.railway) == "platform_marker") {
                Platform p;
                p.setLevel(levelForPlatform((*it).first, e));
                PlatformSection section;
                section.setName(QString::fromUtf8(e.tagValue("ref")));
                section.setPosition(e);
                p.setSections({section});
                m_floatingSections.push_back(std::move(p));
            }

            if (e.type() == OSM::Type::Node) {
                continue;
            }
            const auto railway = e.tagValue(m_tagKeys.railway);
            if (railway == "platform") {
                QRegularExpression splitExp(QStringLiteral("[;,/\\+]"));;
                const auto names = QString::fromUtf8(e.tagValue("local_ref", "ref")).split(splitExp);
                const auto ifopts = e.tagValue("ref:IFOPT").split(';');
                for (auto i = 0; i < names.size(); ++i) {
                    Platform platform;
                    platform.setArea(e);
                    platform.setName(names[i]);
                    platform.setLevel(levelForPlatform((*it).first, e));
                    platform.setMode(modeForElement(e));
                    platform.setSections(sectionsForPath(e.outerPath(m_data.dataSet()), names[i]));
                    if (ifopts.size() == names.size()) {
                        platform.setIfopt(QString::fromUtf8(ifopts[i]));
                    }
                    // we delay merging of platforms, as those without track names would
                    // otherwise cobble together two distinct edges when merged to early
                    m_platformAreas.push_back(std::move(platform));
                }
            }
            else if (railway == "platform_edge" && e.type() == OSM::Type::Way) {
                Platform platform;
                platform.setEdge(e);
                platform.setName(QString::fromUtf8(e.tagValue("local_ref", "ref")));
                platform.setLevel(levelForPlatform((*it).first, e));
                platform.setSections(sectionsForPath(e.outerPath(m_data.dataSet()), platform.name()));
                platform.setIfopt(QString::fromUtf8(e.tagValue("ref:IFOPT")));
                addPlatform(std::move(platform));
            }
            else if (!railway.isEmpty() && e.type() == OSM::Type::Way && railway != "disused") {
                OSM::for_each_node(m_data.dataSet(), *e.way(), [&](const auto &node) {
                    if (!OSM::contains(m_data.boundingBox(), node.coordinate)) {
                        return;
                    }
                    if (OSM::tagValue(node, m_tagKeys.railway) == "buffer_stop") {
                        return;
                    }

                    const auto pt = OSM::tagValue(node, m_tagKeys.public_transport);
                    if (pt == "stop_point" || pt == "stop_position") {
                        Platform platform;
                        platform.setStopPoint(OSM::Element(&node));
                        platform.setTrack({e});
                        platform.setLevel(levelForPlatform((*it).first, e));
                        platform.setName(Platform::preferredName(QString::fromUtf8(platform.stopPoint().tagValue("local_ref", "ref", "name")), nameFromTrack(e)));
                        platform.setMode(modeForElement(OSM::Element(&node)));
                        platform.setIfopt(QString::fromUtf8(platform.stopPoint().tagValue("ref:IFOPT")));
                        if (platform.mode() == Platform::Unknown) {
                            platform.setMode(modeForElement(e));
                        }
                        platform.setSections(sectionsForPath(e.outerPath(m_data.dataSet()), platform.name()));

                        addPlatform(std::move(platform));
                    }
                });
            }
        }
    }

    OSM::for_each(m_data.dataSet(), [this](OSM::Element e) {
        const auto route = e.tagValue(m_tagKeys.route);
        if (route.isEmpty() || route == "tracks") {
            return;
        }
        scanRoute(e, e);
    }, OSM::IncludeRelations);

    mergePlatformAreas();
    for (auto &p : m_floatingSections) {
        addPlatform(std::move(p));
    }
    m_floatingSections.clear();

    finalizeResult();
    return std::move(m_platforms);
}

void PlatformFinder::resolveTagKeys()
{
    m_tagKeys.level = m_data.dataSet().tagKey("level");
    m_tagKeys.platform_ref = m_data.dataSet().tagKey("platform_ref");
    m_tagKeys.platform_colon_ref = m_data.dataSet().tagKey("platform:ref");
    m_tagKeys.public_transport = m_data.dataSet().tagKey("public_transport");
    m_tagKeys.railway = m_data.dataSet().tagKey("railway");
    m_tagKeys.railway_platform_section = m_data.dataSet().tagKey("railway:platform:section");
    m_tagKeys.route = m_data.dataSet().tagKey("route");
    m_tagKeys.disused = m_data.dataSet().tagKey("disused");
}

void PlatformFinder::scanRoute(OSM::Element e, OSM::Element route)
{
    switch (e.type()) {
        case OSM::Type::Null:
            return;
        case OSM::Type::Node:
            scanRoute(*e.node(), route);
            break;
        case OSM::Type::Way:
            OSM::for_each_node(m_data.dataSet(), *e.way(), [this, route](const OSM::Node &node) {
                scanRoute(node, route);
            });
            break;
        case OSM::Type::Relation:
            OSM::for_each_member(m_data.dataSet(), *e.relation(), [this, route](OSM::Element e) {
                scanRoute(e, route);
            });
            break;
    }
}

void PlatformFinder::scanRoute(const OSM::Node& node, OSM::Element route)
{
    const auto pt = OSM::tagValue(node, m_tagKeys.public_transport);
    if (pt.isEmpty()) {
        return;
    }

    for (auto &p : m_platforms) {
        if (p.stopPoint().id() == node.id) {
            const auto l = QString::fromUtf8(route.tagValue("ref")).split(QLatin1Char(';'));
            for (const auto &lineName : l) {
                if (lineName.isEmpty()) {
                    continue;
                }
                auto lines = p.takeLines();
                const auto it = std::lower_bound(lines.begin(), lines.end(), lineName, m_collator);
                if (it == lines.end() || (*it) != lineName) {
                    lines.insert(it, lineName);
                }
                p.setLines(std::move(lines));
            }
            break;
        }
    }
}

std::vector<PlatformSection> PlatformFinder::sectionsForPath(const std::vector<const OSM::Node*> &path, const QString &platformName) const
{
    std::vector<PlatformSection> sections;
    if (path.empty()) {
        return sections;
    }

    // skip the last node for closed paths
    for (auto it = path.begin(); it != (path.front()->id == path.back()->id ? std::prev(path.end()) : path.end()); ++it) {
        const auto n = (*it);
        const auto platformRef = OSM::tagValue(*n, m_tagKeys.platform_ref);
        if (!platformRef.isEmpty() && platformRef != platformName.toUtf8()) {
            continue; // belongs to a different track on the same platform area
        }
        const auto pt = OSM::tagValue(*n, m_tagKeys.public_transport);
        if (pt == "platform_section_sign") {
            PlatformSection sec;
            sec.setPosition(OSM::Element(n));
            sec.setName(QString::fromUtf8(sec.position().tagValue("platform_section_sign_value", "local_ref", "ref", "platform_section_sign")));
            sections.push_back(std::move(sec));
            continue;
        }
        const auto railway_platform_section = OSM::tagValue(*n, m_tagKeys.railway_platform_section);
        if (!railway_platform_section.isEmpty()) {
            PlatformSection sec;
            sec.setPosition(OSM::Element(n));
            sec.setName(QString::fromUtf8(railway_platform_section));
            sections.push_back(std::move(sec));
            continue;
        }
    }
    return sections;
}

struct {
    const char* name;
    Platform::Mode mode;
} static constexpr const mode_map[] = {
    { "rail", Platform::Rail },
    { "subway", Platform::Subway },
    { "tram", Platform::Tram },
    { "light_rail", Platform::Rail }, // TODO consumer code can't handle LightRail yet
    { "monorail", Platform::Tram }, // TODO consumer code can't handle Monorail yet
    { "bus", Platform::Bus },
};

Platform::Mode PlatformFinder::modeForElement(OSM::Element elem) const
{
    const auto railway = elem.tagValue(m_tagKeys.railway);
    if (railway == "tram_stop") {
        return Platform::Tram;
    }

    for (const auto &mode : mode_map) {
        const auto modeTag = elem.tagValue(mode.name);
        if (railway == mode.name || (!modeTag.isEmpty() && modeTag != "no")) {
            return mode.mode;
        }
    }

    // TODO this should eventually return Unknown
    return Platform::Rail;
}

int PlatformFinder::levelForPlatform(const MapLevel &ml, OSM::Element e) const
{
    if (ml.numericLevel() != 0) {
        return qRound(ml.numericLevel() / 10.0) * 10;
    }
    return e.tagValue(m_tagKeys.level).isEmpty() ? std::numeric_limits<int>::min() : 0;
}

void PlatformFinder::addPlatform(Platform &&platform)
{
    for (Platform &p : m_platforms) {
        if (Platform::isSame(p, platform, m_data.dataSet())) {
            p = Platform::merge(p, platform, m_data.dataSet());
            return;
        }
    }

    m_platforms.push_back(std::move(platform));
}

void PlatformFinder::mergePlatformAreas()
{
    // due to split areas we can end up with multplie entries for the same platform that only merge in the right order
    // so retry until we no longer find anything matching
    std::size_t prevCount = 0;

    while (prevCount != m_platformAreas.size() && !m_platformAreas.empty()) {
        prevCount = m_platformAreas.size();
        for (auto it = m_platformAreas.begin(); it != m_platformAreas.end();) {
            bool found = false;
            for (Platform &p : m_platforms) {
                if (Platform::isSame(p, (*it), m_data.dataSet())) {
                    p = Platform::merge(p, (*it), m_data.dataSet());
                    found = true;
                }
            }
            if (found) {
                it = m_platformAreas.erase(it);
            } else {
                ++it;
            }
        }

        if (prevCount == m_platformAreas.size()) {
            m_platforms.push_back(m_platformAreas.back());
            m_platformAreas.erase(std::prev(m_platformAreas.end()));
        }
    }
}

void PlatformFinder::finalizeResult()
{
    if (m_platforms.empty()) {
        return;
    }

    // integrating the platform elements can have made other platforms mergable that previously weren't,
    // the same can happen in case of a very fine-granular track split
    // so do another merge pass over everything we have found so far
    for (auto it = m_platforms.begin(); it != std::prev(m_platforms.end()) && it != m_platforms.end(); ++it) {
        for (auto it2 = std::next(it); it2 != m_platforms.end();) {
            if (Platform::isSame(*it, *it2, m_data.dataSet())) {
                (*it) = Platform::merge(*it, *it2, m_data.dataSet());
                it2 = m_platforms.erase(it2);
            } else {
                ++it2;
            }
        }
    }

    // remove things that are still incomplete at this point
    m_platforms.erase(std::remove_if(m_platforms.begin(), m_platforms.end(), [](const auto &p) {
        return !p.isValid() || p.mode() == Platform::Bus; // ### Bus isn't properly supported yet
    }), m_platforms.end());

    // filter and sort sections on each platform
    for (auto &p : m_platforms) {
        auto sections = p.takeSections();
        sections.erase(std::remove_if(sections.begin(), sections.end(), [](const auto &s) { return !s.isValid(); }), sections.end());
        std::sort(sections.begin(), sections.end(), [this](const auto &lhs, const auto &rhs) {
            return m_collator.compare(lhs.name(), rhs.name()) < 0;
        });
        p.setSections(std::move(sections));
    }

    // sort platforms by mode/name
    std::sort(m_platforms.begin(), m_platforms.end(), [this](const auto &lhs, const auto &rhs) {
        if (lhs.mode() == rhs.mode()) {
            if (lhs.name() == rhs.name()) {
                return lhs.stopPoint().id() < rhs.stopPoint().id();
            }
            return m_collator.compare(lhs.name(), rhs.name()) < 0;
        }
        return lhs.mode() < rhs.mode();
    });
}
