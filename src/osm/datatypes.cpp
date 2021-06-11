/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "datatypes.h"

using namespace OSM;

DataSet::DataSet() = default;
DataSet::DataSet(DataSet &&) = default;
DataSet::~DataSet() = default;

DataSet& DataSet::operator=(DataSet &&) = default;

TagKey DataSet::makeTagKey(const char *keyName, OSM::StringMemory keyMemOpt)
{
    return m_tagKeyRegistry.makeKey(keyName, keyMemOpt);
}

Role DataSet::makeRole(const char *roleName, OSM::StringMemory memOpt)
{
    return m_roleRegistry.makeKey(roleName, memOpt);
}

TagKey DataSet::tagKey(const char *keyName) const
{
    return m_tagKeyRegistry.key(keyName);
}

Role DataSet::role(const char *roleName) const
{
    return m_roleRegistry.key(roleName);
}

const Node* DataSet::node(Id id) const
{
    const auto it = std::lower_bound(nodes.begin(), nodes.end(), id);
    if (it != nodes.end() && (*it).id == id) {
        return &(*it);
    }
    return nullptr;
}

const Way* DataSet::way(Id id) const
{
    const auto it = std::lower_bound(ways.begin(), ways.end(), id);
    if (it != ways.end() && (*it).id == id) {
        return &(*it);
    }
    return nullptr;
}

Way* DataSet::way(Id id)
{
    const auto it = std::lower_bound(ways.begin(), ways.end(), id);
    if (it != ways.end() && (*it).id == id) {
        return &(*it);
    }
    return nullptr;
}

const Relation* DataSet::relation(Id id) const
{
    const auto it = std::lower_bound(relations.begin(), relations.end(), id);
    if (it != relations.end() && (*it).id == id) {
        return &(*it);
    }
    return nullptr;
}

void DataSet::addNode(Node &&node)
{
    const auto it = std::lower_bound(nodes.begin(), nodes.end(), node);
    if (it != nodes.end() && (*it).id == node.id) {
        // do we need to merge something here?
        return;
    }
    nodes.insert(it, std::move(node));
}

void DataSet::addWay(Way &&way)
{
    const auto it = std::lower_bound(ways.begin(), ways.end(), way);
    if (it != ways.end() && (*it).id == way.id) {
        // already there?
        return;
    }
    ways.insert(it, std::move(way));
}

void DataSet::addRelation(Relation &&rel)
{
    const auto it = std::lower_bound(relations.begin(), relations.end(), rel);
    if (it != relations.end() && (*it).id == rel.id) {
        // do we need to merge something here?
        return;
    }
    relations.insert(it, std::move(rel));
}

OSM::Id DataSet::nextInternalId() const
{
    static OSM::Id nextId = 0;
    return --nextId;
}

// resolve ids for elements split in Marble vector tiles
template <typename T>
static QString actualIdString(const T &elem)
{
    const auto mxoid = OSM::tagValue(elem, "mx:oid");
    return mxoid.isEmpty() ? QString::number(elem.id) : QString::fromUtf8(mxoid);
}

QString OSM::Node::url() const
{
    return QStringLiteral("https://openstreetmap.org/node/") + actualIdString(*this);
}

bool OSM::Way::isClosed() const
{
    return nodes.size() >= 2 && nodes.front() == nodes.back();
}

QString OSM::Way::url() const
{
    return QStringLiteral("https://openstreetmap.org/way/") + actualIdString(*this);
}

QString OSM::Relation::url() const
{
    return QStringLiteral("https://openstreetmap.org/relation/") + actualIdString(*this);
}

QDebug operator<<(QDebug debug, OSM::Coordinate coord)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << '(' << coord.latF() << ',' << coord.lonF() << ')';
    return debug;
}

QDebug operator<<(QDebug debug, OSM::BoundingBox bbox)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << '[' << bbox.min.latF() << ',' << bbox.min.lonF() << '|' << bbox.max.latF() << ',' << bbox.max.lonF() << ']';
    return debug;
}
