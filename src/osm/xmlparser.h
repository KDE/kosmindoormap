/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef OSM_XMLPARSER_H
#define OSM_XMLPARSER_H

#include <kosm_export.h>

#include <QString>

class QIODevice;
class QXmlStreamReader;

namespace OSM {

class DataSet;

class KOSM_EXPORT XmlParser
{
public:
    explicit XmlParser(DataSet *dataSet);

    void parse(QIODevice *io);
    QString error() const;

private:
    void parseNode(QXmlStreamReader &reader);
    void parseWay(QXmlStreamReader &reader);
    void parseRelation(QXmlStreamReader &reader);
    template <typename T>
    void parseTag(QXmlStreamReader &reader, T &elem);
    template <typename T>
    void parseTagOrBounds(QXmlStreamReader &reader, T&elem);
    template <typename T>
    void parseBounds(QXmlStreamReader &reader, T &elem);

    DataSet *m_dataSet;
    QString m_error;
};

}

#endif // OSM_XMLPARSER_P_H
