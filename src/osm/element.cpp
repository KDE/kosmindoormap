/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "element.h"
#include "pathutil.h"

using namespace OSM;

Id Element::id() const
{
    switch (type()) {
        case Type::Null:
            return {};
        case Type::Node:
            return node()->id;
        case Type::Way:
            return way()->id;
        case Type::Relation:
            return relation()->id;
    }

    Q_UNREACHABLE();
    return {};
}

Coordinate Element::center() const
{
    switch (type()) {
        case Type::Null:
            return {};
        case Type::Node:
            return node()->coordinate;
        case Type::Way:
            return way()->bbox.center();
        case Type::Relation:
            return relation()->bbox.center();
    }

    Q_UNREACHABLE();
    return {};
}

BoundingBox Element::boundingBox() const
{
    switch (type()) {
        case Type::Null:
            return {};
        case Type::Node:
            return BoundingBox(node()->coordinate, node()->coordinate);
        case Type::Way:
            return way()->bbox;
        case Type::Relation:
            return relation()->bbox;
    }

    Q_UNREACHABLE();
    return {};
}

QByteArray Element::tagValue(TagKey key) const
{
    switch (type()) {
        case Type::Null:
            return {};
        case Type::Node:
            return OSM::tagValue(*node(), key);
        case Type::Way:
            return OSM::tagValue(*way(), key);
        case Type::Relation:
            return OSM::tagValue(*relation(), key);
    }

    Q_UNREACHABLE();
    return {};
}

QByteArray Element::tagValue(const char *keyName) const
{
    switch (type()) {
        case Type::Null:
            return {};
        case Type::Node:
            return OSM::tagValue(*node(), keyName);
        case Type::Way:
            return OSM::tagValue(*way(), keyName);
        case Type::Relation:
            return OSM::tagValue(*relation(), keyName);
    }

    Q_UNREACHABLE();
    return {};
}

QByteArray Element::tagValue(const OSM::Languages &languages, const char *keyName) const
{
    switch (type()) {
        case Type::Null:
            return {};
        case Type::Node:
            return OSM::tagValue(*node(), languages, keyName);
        case Type::Way:
            return OSM::tagValue(*way(), languages, keyName);
        case Type::Relation:
            return OSM::tagValue(*relation(), languages, keyName);
    }

    Q_UNREACHABLE();
    return {};
}

bool Element::hasTag(TagKey key) const
{
    return std::binary_search(tagsBegin(), tagsEnd(), key);
}

std::vector<Tag>::const_iterator OSM::Element::tagsBegin() const
{
    switch (type()) {
        case Type::Null:
            Q_UNREACHABLE();
        case Type::Node:
            return node()->tags.begin();
        case Type::Way:
            return way()->tags.begin();
        case Type::Relation:
            return relation()->tags.begin();
    }
    Q_UNREACHABLE();
}

std::vector<Tag>::const_iterator OSM::Element::tagsEnd() const
{
    switch (type()) {
        case Type::Null:
            Q_UNREACHABLE();
        case Type::Node:
            return node()->tags.end();
        case Type::Way:
            return way()->tags.end();
        case Type::Relation:
            return relation()->tags.end();
    }
    Q_UNREACHABLE();
}

QString Element::url() const
{
    switch (type()) {
        case Type::Null:
            return {};
        case Type::Node:
            return node()->url();
        case Type::Way:
            return way()->url();
        case Type::Relation:
            return relation()->url();
    }

    Q_UNREACHABLE();
    return {};
}

std::vector<const Node*> Element::outerPath(const DataSet &dataSet) const
{
    switch (type()) {
        case Type::Null:
            return {};
        case Type::Node:
            return {node()};
        case Type::Way:
        {
            std::vector<const Node*> nodes;
            appendNodesFromWay(dataSet, nodes, way()->nodes.begin(), way()->nodes.end());
            return nodes;
        }
        case Type::Relation:
        {
            if (tagValue("type") != "multipolygon") {
                return {};
            }

            // collect the relevant ways
            std::vector<const Way*> ways;
            for (const auto &member : relation()->members) {
                if (std::strcmp(member.role().name(), "outer") != 0) {
                    continue;
                }
                if (auto way = dataSet.way(member.id)) {
                    ways.push_back(way);
                }
            }

            // stitch them together (there is no well-defined order)
            std::vector<const Node*> nodes;
            assemblePath(dataSet, std::move(ways), nodes);
            return nodes;
        }
    }

    Q_UNREACHABLE();
    return {};
}

void Element::recomputeBoundingBox(const DataSet &dataSet)
{
    switch (type()) {
        case Type::Null:
        case Type::Node:
            break;
        case Type::Way:
            way()->bbox = std::accumulate(way()->nodes.begin(), way()->nodes.end(), OSM::BoundingBox(), [&dataSet](auto bbox, auto nodeId) {
                if (auto node = dataSet.node(nodeId)) {
                    return OSM::unite(bbox, {node->coordinate, node->coordinate});
                }
                return bbox;
            });
            break;
        case Type::Relation:
            relation()->bbox = {};
            for_each_member(dataSet, *relation(), [this, &dataSet](auto mem) {
                mem.recomputeBoundingBox(dataSet);
                relation()->bbox = OSM::unite(relation()->bbox, mem.boundingBox());
            });
            break;
    }
}


UniqueElement::~UniqueElement()
{
    switch (m_element.type()) {
        case OSM::Type::Null:
            break;
        case OSM::Type::Node:
            delete m_element.node();
            break;
        case OSM::Type::Way:
            delete m_element.way();
            break;
        case OSM::Type::Relation:
            delete m_element.relation();
            break;
    }
}

void UniqueElement::setId(Id id)
{
    switch (m_element.type()) {
        case OSM::Type::Null:
            return;
        case OSM::Type::Node:
            const_cast<Node*>(m_element.node())->id = id;
            break;
        case OSM::Type::Way:
            const_cast<Way*>(m_element.way())->id = id;
            break;
        case OSM::Type::Relation:
            const_cast<Relation*>(m_element.relation())->id = id;
            break;
    }
}

void UniqueElement::setTagValue(TagKey key, QByteArray &&value)
{
    switch (m_element.type()) {
        case OSM::Type::Null:
            return;
        case OSM::Type::Node:
            OSM::setTagValue(*const_cast<Node*>(m_element.node()), key, std::move(value));
            break;
        case OSM::Type::Way:
            OSM::setTagValue(*const_cast<Way*>(m_element.way()), key, std::move(value));
            break;
        case OSM::Type::Relation:
            OSM::setTagValue(*const_cast<Relation*>(m_element.relation()), key, std::move(value));
            break;
    }
}

void UniqueElement::removeTag(TagKey key)
{
    switch (m_element.type()) {
        case OSM::Type::Null:
            return;
        case OSM::Type::Node:
            OSM::removeTag(*const_cast<Node*>(m_element.node()), key);
            break;
        case OSM::Type::Way:
            OSM::removeTag(*const_cast<Way*>(m_element.way()), key);
            break;
        case OSM::Type::Relation:
            OSM::removeTag(*const_cast<Relation*>(m_element.relation()), key);
            break;
    }
}

UniqueElement OSM::copy_element(Element e)
{
    switch (e.type()) {
        case OSM::Type::Null:
            return UniqueElement();
        case OSM::Type::Node:
            return UniqueElement(new Node(*e.node()));
        case OSM::Type::Way:
            return UniqueElement(new Way(*e.way()));
        case OSM::Type::Relation:
            return UniqueElement(new Relation(*e.relation()));
    }
    Q_UNREACHABLE();
    return UniqueElement();
}
