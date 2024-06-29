/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef OSM_OSCPARSER_H
#define OSM_OSCPARSER_H

#include "xmlparser.h"
#include "datatypes.h"

#include <unordered_map>

namespace OSM {

/** Parser for the OSM change format.
 *  @see https://wiki.openstreetmap.org/wiki/OsmChange
 */
class OscParser : public XmlParser
{
public:
    explicit OscParser(DataSet *dataSet);

private:
    void readFromIODevice(QIODevice *io) override;
    void parseCreate(QXmlStreamReader &reader);
    void parseModify(QXmlStreamReader &reader);
    void parseDelete(QXmlStreamReader &reader);

    template <typename T>
    void assignNewId(T &elem, std::unordered_map<OSM::Id, OSM::Id> &idMap);
    [[nodiscard]] static OSM::Id mapId(OSM::Id id, const std::unordered_map<OSM::Id, OSM::Id> &idMap);
    /** Remap all way node ids. */
    void mapNodeIds(OSM::Way &way) const;
    /** Remap all member ids. */
    void mapMemberIds(OSM::Relation &rel) const;

    std::unordered_map<OSM::Id, OSM::Id> m_nodeIdMap;
    std::unordered_map<OSM::Id, OSM::Id> m_wayIdMap;
    std::unordered_map<OSM::Id, OSM::Id> m_relIdMap;
};

}

#endif // OSM_OSCPARSER_P_H
