/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef OSM_XMLPARSER_H
#define OSM_XMLPARSER_H

#include "abstractreader.h"

#include <QString>

class QIODevice;
class QXmlStreamReader;

namespace OSM {

class DataSet;

class XmlParser : public AbstractReader
{
public:
    explicit XmlParser(DataSet *dataSet);

protected: // for reuse by the OSC parser
    [[nodiscard]] OSM::Node parseNode(QXmlStreamReader &reader) const;
    [[nodiscard]] OSM::Way parseWay(QXmlStreamReader &reader) const;
    [[nodiscard]] OSM::Relation parseRelation(QXmlStreamReader &reader) const;

private:
    void readFromIODevice(QIODevice *io) override;

    template <typename T>
    void parseTag(QXmlStreamReader &reader, T &elem) const;
    template <typename T>
    void parseTagOrBounds(QXmlStreamReader &reader, T&elem) const;
    template <typename T>
    void parseBounds(QXmlStreamReader &reader, T &elem) const;
};

}

#endif // OSM_XMLPARSER_P_H
