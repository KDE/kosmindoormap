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
        reader.readNextStartElement();
        if (reader.name() == "create"_L1) {
            parseCreate(reader);
        } else if (reader.name() == "modify"_L1) {
            // TODO
            qWarning() << "element modification is not implemented yet!";
            reader.skipCurrentElement();
        } else if (reader.name() == "delete"_L1){
            qWarning() << "element deletion is not supported yet!";
            reader.skipCurrentElement();
        } else {
            reader.skipCurrentElement();
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

void OscParser::parseCreate(QXmlStreamReader &reader)
{
    while (!reader.atEnd() && !reader.hasError()) {
        reader.readNextStartElement();
        if (reader.name() == "node"_L1) {
            auto node = parseNode(reader);
            assignNewId(node, m_nodeIdMap);
            addNode(std::move(node));
        } else if (reader.name() == "way"_L1) {
            auto way = parseWay(reader);
            assignNewId(way, m_wayIdMap);
            for (auto &id : way.nodes) {
                id = mapId(id, m_nodeIdMap);
            }
            addWay(std::move(way));
        } else if (reader.name() == "releation"_L1) {
            auto rel = parseRelation(reader);
            assignNewId(rel, m_relIdMap);
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
            addRelation(std::move(rel));
        } else {
            reader.skipCurrentElement();
        }
    }
}
