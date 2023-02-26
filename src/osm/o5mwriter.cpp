/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "o5mwriter.h"
#include "o5m.h"
#include "datatypes.h"

#include <QBuffer>
#include <QIODevice>

using namespace OSM;

static void writeByte(uint8_t b, QIODevice *io)
{
    io->write(reinterpret_cast<const char*>(&b), 1);
}

static void writeUnsigned(uint64_t n, QIODevice *io)
{
    do {
        uint8_t b = ((n >> 7) > 0 ? O5M_NUMBER_CONTINUATION : 0) | (n & O5M_NUMBER_MASK);
        writeByte(b, io);
        n >>= 7;
    } while (n > 0);
}

static void writeSigned(int64_t n, QIODevice *io)
{
    uint64_t u = n < 0 ? (-n - 1) : n;
    u <<= 1;
    u |= n < 0 ? O5M_NUMBER_SIGNED_BIT : 0;
    writeUnsigned(u, io);
}

static void writeHeader(QIODevice *io)
{
    writeByte(O5M_BLOCK_RESET, io);
    writeByte(O5M_BLOCK_HEADER, io);
    writeByte(4, io);
    io->write(O5M_HEADER, 4);
}

static void writeTrailer(QIODevice *io)
{
    writeByte(O5M_TRAILER, io);
}

void O5mWriter::writeToIODevice(const OSM::DataSet& dataSet, QIODevice* io)
{
    writeHeader(io);
    writeNodes(dataSet, io);
    writeWays(dataSet, io);
    writeRelations(dataSet, io);
    writeTrailer(io);

    m_stringTable.clear();
}

void O5mWriter::writeNodes(const OSM::DataSet &dataSet, QIODevice *io)
{
    if (dataSet.nodes.empty()) {
        return;
    }

    writeByte(O5M_BLOCK_RESET, io);
    m_stringTable.clear();

    OSM::Id prevId = 0;
    int64_t prevLat = 900'000'000ll;
    int64_t prevLon = 1'800'000'000ll;

    QByteArray bufferData;
    QBuffer buffer(&bufferData);
    for(auto const &node: dataSet.nodes) {
        bufferData.clear();
        buffer.open(QIODevice::WriteOnly);
        writeByte(O5M_BLOCK_NODE, io);

        writeSigned((int64_t)node.id - (int64_t)prevId, &buffer);
        prevId = node.id;

        writeByte(0x0, &buffer);
        writeSigned((int64_t)node.coordinate.longitude - prevLon, &buffer);
        prevLon = node.coordinate.longitude;
        writeSigned((int64_t)node.coordinate.latitude - prevLat, &buffer);
        prevLat = node.coordinate.latitude;

        writeTags(node, &buffer);

        buffer.close();
        writeUnsigned(bufferData.size(), io);
        io->write(bufferData.constData(), bufferData.size());
    }
}

void O5mWriter::writeWays(const OSM::DataSet &dataSet, QIODevice *io)
{
    if (dataSet.ways.empty()) {
        return;
    }

    writeByte(O5M_BLOCK_RESET, io);
    m_stringTable.clear();
    OSM::Id prevId = 0;
    OSM::Id prevNodeId = 0;

    QByteArray bufferData;
    QBuffer buffer(&bufferData);
    QByteArray referencesBufferData;
    QBuffer referencesBuffer(&referencesBufferData);

    for (auto const &way: dataSet.ways) {
        writeByte(O5M_BLOCK_WAY, io);

        bufferData.clear();
        buffer.open(QIODevice::WriteOnly);
        writeSigned((int64_t)way.id - (int64_t)prevId, &buffer);
        prevId = way.id;
        writeByte(0x0, &buffer);

        referencesBufferData.clear();
        referencesBuffer.open(QIODevice::WriteOnly);
        for (const auto &node : way.nodes) {
            writeSigned((int64_t)node - (int64_t)prevNodeId, &referencesBuffer);
            prevNodeId = node;
        }
        referencesBuffer.close();
        writeUnsigned(referencesBufferData.size(), &buffer);
        buffer.write(referencesBufferData.constData(), referencesBufferData.size());
        writeTags(way, &buffer);
        buffer.close();
        writeUnsigned(bufferData.size(), io);
        io->write(bufferData.constData(), bufferData.size());
    }
}

void O5mWriter::writeRelations(const OSM::DataSet &dataSet, QIODevice *io)
{
    if (dataSet.relations.empty()) {
        return;
    }

    writeByte(O5M_BLOCK_RESET, io);
    m_stringTable.clear();
    OSM::Id prevId = 0;
    OSM::Id prevMemberId[3] = { 0, 0, 0 };

    QByteArray bufferData;
    QBuffer buffer(&bufferData);
    QByteArray referencesBufferData;
    QBuffer referencesBuffer(&referencesBufferData);
    QByteArray role;

    for (auto const &relation: dataSet.relations) {
        writeByte(O5M_BLOCK_RELATION, io);

        bufferData.clear();
        buffer.open(QIODevice::WriteOnly);

        writeSigned((int64_t)relation.id - (int64_t)prevId, &buffer);
        prevId = relation.id;
        writeByte(0x0, &buffer);

        referencesBufferData.clear();
        referencesBuffer.open(QIODevice::WriteOnly);

        for (const auto &member : relation.members) {
            role.clear();
            switch (member.type()) {
                case OSM::Type::Node:
                    writeSigned((int64_t)member.id - (int64_t)prevMemberId[0], &referencesBuffer);
                    prevMemberId[0] = member.id;
                    role += (const char)O5M_MEMTYPE_NODE;
                    role += member.role().name();
                    writeStringPair(role.constData(), nullptr, &referencesBuffer);
                    break;
                case OSM::Type::Way:
                    writeSigned((int64_t)member.id - (int64_t)prevMemberId[1], &referencesBuffer);
                    prevMemberId[1] = member.id;
                    role += (const char)O5M_MEMTYPE_WAY;
                    role += member.role().name();
                    writeStringPair(role.constData(), nullptr, &referencesBuffer);
                    break;
                case OSM::Type::Relation:
                    writeSigned((int64_t)member.id - (int64_t)prevMemberId[2], &referencesBuffer);
                    prevMemberId[2] = member.id;
                    role += (const char)O5M_MEMTYPE_RELATION;
                    role += member.role().name();
                    writeStringPair(role.constData(), nullptr, &referencesBuffer);
                    break;
                case OSM::Type::Null:
                    assert(false);
                    break;
            }
        }

        referencesBuffer.close();
        writeUnsigned(referencesBufferData.size(), &buffer);
        buffer.write(referencesBufferData.constData(), referencesBufferData.size());
        writeTags(relation, &buffer);
        buffer.close();
        writeUnsigned(bufferData.size(), io);
        io->write(bufferData.constData(), bufferData.size());
    }
}

template <typename T>
void O5mWriter::writeTags(const T &elem, QIODevice *io)
{
    for (auto &tag : elem.tags) {
        writeStringPair(tag.key.name(), tag.value.constData(), io);
    }
}

void O5mWriter::writeStringPair(const char *s1, const char *s2, QIODevice *io)
{
    assert(s1);
    O5mStringPair p;
    p.s1 = s1;
    if (s2) {
        p.s2 = s2;
    }
    const auto it = m_stringTable.find(p);
    if (it != m_stringTable.end()) {
        writeUnsigned(m_stringTable.size() - it->second, io);
    } else {
        writeByte(0x0, io);
        io->write(s1);
        writeByte(0x0, io);
        if (s2) {
            io->write(s2);
            writeByte(0x0, io);
        }
        if ((std::strlen(s1) + (s2 ? std::strlen(s2) : 0) <= O5M_STRING_TABLE_MAXLEN) && (m_stringTable.size() <= O5M_STRING_TABLE_SIZE)) {
            m_stringTable[p] = m_stringTable.size();
        }
    }
}
