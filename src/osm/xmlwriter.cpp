/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "xmlwriter.h"
#include "datatypes.h"
#include "element.h"

#include <QXmlStreamWriter>

using namespace Qt::Literals::StringLiterals;
using namespace OSM;

template <typename T>
static void writeTags(QXmlStreamWriter &writer, const T &elem)
{
    for (const auto &tag : elem.tags) {
        writer.writeStartElement(QStringLiteral("tag"));
        writer.writeAttribute(QStringLiteral("k"), QString::fromUtf8(tag.key.name()));
        writer.writeAttribute(QStringLiteral("v"), QString::fromUtf8(tag.value));
        writer.writeEndElement();
    }
}

void XmlWriter::writeToIODevice(const DataSet &dataSet, QIODevice *out)
{
    QXmlStreamWriter writer(out);
    writer.setAutoFormatting(true);
    writer.setAutoFormattingIndent(-1);

    writer.writeStartDocument();
    // using \123 instead of 'S' here prevents reuse lint from interpreting the following string as a license header for this file
    writer.writeComment(QStringLiteral("\n    \123PDX-FileCopyrightText: OpenStreetMap contributors\n    \123PDX-License-Identifier: ODbL-1.0\n"));
    writer.writeStartElement(QStringLiteral("osm"));
    writer.writeAttribute(QStringLiteral("version"), QStringLiteral("0.6"));
    writer.writeAttribute(QStringLiteral("generator"), QStringLiteral("KOSM"));

    OSM::BoundingBox bbox;
    OSM::for_each(dataSet, [&bbox](auto elem) { bbox = OSM::unite(bbox, elem.boundingBox()); });
    writer.writeStartElement(QStringLiteral("bounds"));
    writer.writeAttribute(QStringLiteral("minlat"), QString::number(bbox.min.latF(), 'f', 10));;
    writer.writeAttribute(QStringLiteral("minlon"), QString::number(bbox.min.lonF(), 'f', 10));;
    writer.writeAttribute(QStringLiteral("maxlat"), QString::number(bbox.max.latF(), 'f', 10));;
    writer.writeAttribute(QStringLiteral("maxlon"), QString::number(bbox.max.lonF(), 'f', 10));;
    writer.writeEndElement();

    for (const auto &node : dataSet.nodes) {
        writer.writeStartElement(QStringLiteral("node"));
        writer.writeAttribute(QStringLiteral("id"), QString::number(node.id));
        writer.writeAttribute(QStringLiteral("lat"), QString::number(node.coordinate.latF(), 'f', 10));
        writer.writeAttribute(QStringLiteral("lon"), QString::number(node.coordinate.lonF(), 'f', 10));
        writeTags(writer, node);
        writer.writeEndElement();
    }

    for (const auto &way : dataSet.ways) {
        writer.writeStartElement(QStringLiteral("way"));
        writer.writeAttribute(QStringLiteral("id"), QString::number(way.id));
        for (const auto &nd : way.nodes) {
            writer.writeStartElement(QStringLiteral("nd"));
            writer.writeAttribute(QStringLiteral("ref"), QString::number(nd));
            writer.writeEndElement();
        }
        writeTags(writer, way);
        writer.writeEndElement();
    }

    for (const auto &rel : dataSet.relations) {
        writer.writeStartElement(QStringLiteral("relation"));
        writer.writeAttribute(QStringLiteral("id"), QString::number(rel.id));
        for (const auto &mem : rel.members) {
            writer.writeStartElement(QStringLiteral("member"));
            writer.writeAttribute(u"type"_s, QLatin1StringView(OSM::typeName(mem.type())));
            writer.writeAttribute(QStringLiteral("ref"), QString::number(mem.id));
            if (!mem.role().isNull()) {
                writer.writeAttribute(QStringLiteral("role"), QString::fromUtf8(mem.role().name()));
            }
            writer.writeEndElement();
        }
        writeTags(writer, rel);
        writer.writeEndElement();
    }

    writer.writeEndElement();
    writer.writeEndDocument();
}
