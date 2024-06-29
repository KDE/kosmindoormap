/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "oscparser.h"
#include "datatypes.h"

#include <QDebug>
#include <QIODevice>
#include <QXmlStreamReader>

#include <cmath>

using namespace Qt::Literals::StringLiterals;
using namespace OSM;

OscParser::OscParser(DataSet* dataSet)
    : XmlParser(dataSet)
{
}

void OscParser::readFromIODevice(QIODevice *io)
{
    QXmlStreamReader reader(io);
    while (!reader.atEnd() && !reader.hasError()) {
        reader.readNext();
        if (reader.tokenType() != QXmlStreamReader::StartElement) {
            continue;
        }
        if (reader.name() == "create"_L1) {
            parseCreate(reader);
        } else if (reader.name() == "modify"_L1) {
            parseModify(reader);
        } else if (reader.name() == "delete"_L1){
            parseDelete(reader);
        }
    }

    if (reader.hasError()) {
        m_error = reader.errorString();
    }
}

template <typename T>
void OscParser::assignNewId(T &elem, std::unordered_map<OSM::Id, OSM::Id> &idMap)
{
    const auto newId = m_dataSet->nextInternalId();
    idMap[elem.id] = newId;
    elem.id = newId;
}

OSM::Id OscParser::mapId(OSM::Id id, const std::unordered_map<OSM::Id, OSM::Id> &idMap)
{
    auto it = idMap.find(id);
    return it == idMap.end() ? id : (*it).second;
}

void OscParser::mapNodeIds(OSM::Way &way) const
{
    for (auto &id : way.nodes) {
        id = mapId(id, m_nodeIdMap);
    }
}

void OscParser::mapMemberIds(OSM::Relation &rel) const
{
    for (auto &member : rel.members) {
        switch (member.type()) {
            case OSM::Type::Null:
                break;
            case OSM::Type::Node:
                member.id = mapId(member.id, m_nodeIdMap);
                break;
            case OSM::Type::Way:
                member.id = mapId(member.id, m_wayIdMap);
                break;
            case OSM::Type::Relation:
                member.id = mapId(member.id, m_relIdMap);
                break;
        }
    }
}

void OscParser::parseCreate(QXmlStreamReader &reader)
{
    while (!reader.atEnd() && !reader.hasError()) {
        reader.readNext();
        if (reader.tokenType() == QXmlStreamReader::EndElement && reader.name() == "create"_L1) {
            return;
        }
        if (reader.tokenType() != QXmlStreamReader::StartElement) {
            continue;
        }
        if (reader.name() == "node"_L1) {
            auto node = parseNode(reader);
            assignNewId(node, m_nodeIdMap);
            addNode(std::move(node));
        } else if (reader.name() == "way"_L1) {
            auto way = parseWay(reader);
            assignNewId(way, m_wayIdMap);
            mapNodeIds(way);
            addWay(std::move(way));
        } else if (reader.name() == "releation"_L1) {
            auto rel = parseRelation(reader);
            assignNewId(rel, m_relIdMap);
            mapMemberIds(rel);
            addRelation(std::move(rel));
        } else {
            reader.skipCurrentElement();
        }
    }
}

void OscParser::parseModify(QXmlStreamReader &reader)
{
    while (!reader.atEnd() && !reader.hasError()) {
        reader.readNext();
        if (reader.tokenType() == QXmlStreamReader::EndElement && reader.name() == "modify"_L1) {
            return;
        }
        if (reader.tokenType() != QXmlStreamReader::StartElement) {
            continue;
        }
        if (reader.name() == "node"_L1) {
            auto modifiedNode = parseNode(reader);
            if (const auto it = std::lower_bound(m_dataSet->nodes.begin(), m_dataSet->nodes.end(), modifiedNode.id); it != m_dataSet->nodes.end() && (*it).id == modifiedNode.id) {
                if (modifiedNode.coordinate.isValid()) {
                    (*it).coordinate = modifiedNode.coordinate;
                }
                if (!modifiedNode.tags.empty()) {
                    (*it).tags = std::move(modifiedNode.tags);
                }
            } else {
                qDebug() << "modified node not in data set:" << modifiedNode.url();
            }
        } else if (reader.name() == "way"_L1) {
            auto modifiedWay = parseWay(reader);
            if (const auto it = std::lower_bound(m_dataSet->ways.begin(), m_dataSet->ways.end(), modifiedWay.id); it != m_dataSet->ways.end() && (*it).id == modifiedWay.id) {
                if (!modifiedWay.tags.empty()) {
                    (*it).tags = std::move(modifiedWay.tags);
                }
                if (!modifiedWay.nodes.empty()) {
                    mapNodeIds(modifiedWay);
                    (*it).nodes = std::move(modifiedWay.nodes);
                }
            } else {
                qDebug() << "modified way not in data set:" << modifiedWay.url();
            }
        } else if (reader.name() == "releation"_L1) {
            auto modifiedRel = parseRelation(reader);
            if (const auto it = std::lower_bound(m_dataSet->relations.begin(), m_dataSet->relations.end(), modifiedRel.id); it != m_dataSet->relations.end() && (*it).id == modifiedRel.id) {
                if (!modifiedRel.tags.empty()) {
                    (*it).tags = std::move(modifiedRel.tags);
                }
                if (!modifiedRel.members.empty()) {
                    mapMemberIds(modifiedRel);
                    (*it).members = std::move(modifiedRel.members);
                }
            } else {
                qDebug() << "modified relation not in data set:" << modifiedRel.url();
            }
        } else {
            reader.skipCurrentElement();
        }
    }
}

void OscParser::parseDelete(QXmlStreamReader &reader)
{
    // we don't actually delete but just drop all tags
    // this avoids having to deal with broken referential integrity
    // but nevertheless results in the deleted element having not effect anymore
    while (!reader.atEnd() && !reader.hasError()) {
        reader.readNext();
        if (reader.tokenType() == QXmlStreamReader::EndElement && reader.name() == "modify"_L1) {
            return;
        }
        if (reader.tokenType() != QXmlStreamReader::StartElement) {
            continue;
        }
        if (reader.name() == "node"_L1) {
            const auto node = parseNode(reader);
            if (const auto it = std::lower_bound(m_dataSet->nodes.begin(), m_dataSet->nodes.end(), node.id); it != m_dataSet->nodes.end() && (*it).id == node.id) {
                (*it).tags.clear();
            } else {
                qDebug() << "deleted node not in data set:" << node.url();
            }
        } else if (reader.name() == "way"_L1) {
            const auto way = parseWay(reader);
            if (const auto it = std::lower_bound(m_dataSet->ways.begin(), m_dataSet->ways.end(), way.id); it != m_dataSet->ways.end() && (*it).id == way.id) {
                (*it).tags.clear();
            } else {
                qDebug() << "modified way not in data set:" << way.url();
            }
        } else if (reader.name() == "releation"_L1) {
            const auto rel = parseRelation(reader);
            if (const auto it = std::lower_bound(m_dataSet->relations.begin(), m_dataSet->relations.end(), rel.id); it != m_dataSet->relations.end() && (*it).id == rel.id) {
                (*it).tags.clear();
            } else {
                qDebug() << "modified relation not in data set:" << rel.url();
            }
        } else {
            reader.skipCurrentElement();
        }
    }
}
