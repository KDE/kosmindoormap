/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "o5mparser.h"
#include "o5m.h"
#include "datatypes.h"
#include "datasetmergebuffer.h"

#include <QDebug>

#include <cstdlib>
#include <cstring>

using namespace OSM;

O5mParser::O5mParser(DataSet *dataSet)
    : AbstractReader(dataSet)
{
    m_stringLookupTable.resize(O5M_STRING_TABLE_SIZE);
}

void O5mParser::readFromData(const uint8_t* data, std::size_t len)
{
    std::fill(m_stringLookupTable.begin(), m_stringLookupTable.end(), nullptr);
    resetDeltaCodingState();

    const auto endIt = data + len;
    for (auto it = data; it < endIt - 1;) {
        const auto blockType = (*it);
        if (blockType == O5M_BLOCK_RESET) {
            resetDeltaCodingState();
            ++it;
            continue;
        }

        auto blockSize = readUnsigned(++it, endIt);
        if (blockSize >= (uint64_t)(endIt - it)) {
            qWarning() << "premature end of file, or blocksize too large" << (endIt - it) << blockType << blockSize;
            break;
        }
        switch (blockType) {
            case O5M_BLOCK_HEADER:
                if (blockSize != 4 || std::strncmp(reinterpret_cast<const char*>(it), O5M_HEADER, 4) != 0) {
                    qWarning() << "Invalid file header";
                    return;
                }
                break;
            case O5M_BLOCK_BOUNDING_BOX:
            case O5M_BLOCK_TIMESTAMP:
                // not of interest at the moment
                break;
            case O5M_BLOCK_NODE:
                readNode(it, it + blockSize);
                break;
            case O5M_BLOCK_WAY:
                readWay(it, it + blockSize);
                break;
            case O5M_BLOCK_RELATION:
                readRelation(it, it + blockSize);
                break;
            default:
                qDebug() << "unhandled o5m block type:" << (it - data) << blockType << blockSize;
        }

        it += blockSize;
    }
}

uint64_t O5mParser::readUnsigned(const uint8_t *&it, const uint8_t *endIt) const
{
    uint64_t result = 0;
    int i = 0;
    for (; it < endIt && ((*it) & O5M_NUMBER_CONTINUATION); ++it, ++i) {
        result |= ((*it) & O5M_NUMBER_MASK) << (i * 7);
    }
    result |= ((uint64_t)(*it++) & O5M_NUMBER_MASK) << (i * 7);
    return result;
}

int64_t O5mParser::readSigned(const uint8_t *&it, const uint8_t *endIt) const
{
    const uint64_t u = readUnsigned(it, endIt);
    return (u & O5M_NUMBER_SIGNED_BIT) ? (-(u >> 1) -1) : (u >> 1);
}

template <typename T>
T O5mParser::readDelta(const uint8_t *&it, const uint8_t *endIt, T &deltaState)
{
    deltaState += (T)readSigned(it, endIt);
    return deltaState;
}

const char* O5mParser::readString(const uint8_t *&it, const uint8_t *endIt)
{
    auto ref = readUnsigned(it, endIt);
    if (ref) {
        return m_stringLookupTable[(m_stringLookupPosition + O5M_STRING_TABLE_SIZE - ref) % O5M_STRING_TABLE_SIZE];
    } else {
        const auto s = reinterpret_cast<const char*>(it);
        const auto len = std::strlen(s);
        if (len <= O5M_STRING_TABLE_MAXLEN) {
            m_stringLookupTable[m_stringLookupPosition] = s;
            m_stringLookupPosition = (m_stringLookupPosition + 1) % O5M_STRING_TABLE_SIZE;
        }
        it += len + 1;
        return s;
    }
}

std::pair<const char*, const char*> O5mParser::readStringPair(const uint8_t *&it, const uint8_t *endIt)
{
    auto ref = readUnsigned(it, endIt);
    if (ref) {
        const auto s = m_stringLookupTable[(m_stringLookupPosition + O5M_STRING_TABLE_SIZE - ref) % O5M_STRING_TABLE_SIZE];
        if (!s) {
            return {};
        }
        const auto len1 = std::strlen(s);
        return std::make_pair(s, s + len1 + 1);
    } else {
        const auto s = reinterpret_cast<const char*>(it);
        const auto len1 = std::strlen(s);
        const auto len2 = std::strlen(s + len1 + 1);

        if (len1 + len2 <= O5M_STRING_TABLE_MAXLEN) {
            m_stringLookupTable[m_stringLookupPosition] = s;
            m_stringLookupPosition = (m_stringLookupPosition + 1) % O5M_STRING_TABLE_SIZE;
        }

        it += len1 + len2 + 2;
        return std::make_pair(s, s + len1 + 1);
    }
}

void O5mParser::skipVersionInformation(const uint8_t *&it, const uint8_t *end)
{
    if (it >= end) { return; }
    const auto version = readUnsigned(it, end);
    if (version > 0) {
        qWarning() << "skipping changeset data not implemented yet!";
        //    timestamp (seconds since 1970, signed, delta-coded)
        //    author information – only if timestamp is not 0:
        //        changeset (signed, delta-coded)
        //        uid, user (string pair)
        it = end;
    }
}

template<typename Elem>
void O5mParser::readTagOrBbox(Elem &e, const uint8_t *&it, const uint8_t *endIt)
{
    const auto tagData = readStringPair(it, endIt);
    if (!tagData.first) {
        return;
    }
    if (std::strcmp(tagData.first, "bBox") == 0) {
        char *next = nullptr;
        const auto lon1 = std::strtod(tagData.second, &next);
        ++next;
        const auto lat1 = std::strtod(next, &next);
        ++next;
        const auto lon2 = std::strtod(next, &next);
        ++next;
        const auto lat2 = std::strtod(next, &next);
        e.bbox = OSM::BoundingBox(OSM::Coordinate(lat1, lon1), OSM::Coordinate(lat2, lon2));
        return;
    }

    OSM::Tag tag;
    tag.key = m_dataSet->makeTagKey(tagData.first, OSM::StringMemory::Transient); // TODO make use of mmap'ed data for this
    tag.value = QByteArray(tagData.second);
    e.tags.push_back(std::move(tag));
}

void O5mParser::readNode(const uint8_t *begin, const uint8_t *end)
{
    OSM::Node node;

    auto it = begin;
    node.id = readDelta(it, end, m_nodeIdDelta);
    skipVersionInformation(it, end);
    if (it >= end) { return; }

    node.coordinate.longitude = (int64_t)readDelta(it, end, m_lonDelta) + 1'800'000'000ll;
    node.coordinate.latitude = (int64_t)readDelta(it, end, m_latDelata) + 900'000'000ll;

    while (it < end) {
        OSM::Tag tag;
        const auto tagData = readStringPair(it, end);
        if (tagData.first) {
            tag.key = m_dataSet->makeTagKey(tagData.first, OSM::StringMemory::Transient); // TODO use the fact this is mmap'ed data here
            tag.value = QByteArray(tagData.second);
            node.tags.push_back(std::move(tag));
        }
    }
    std::sort(node.tags.begin(), node.tags.end());

    addNode(std::move(node));
}

void O5mParser::readWay(const uint8_t *begin, const uint8_t *end)
{
    OSM::Way way;

    auto it = begin;
    way.id = readDelta(it, end, m_wayIdDelta);
    skipVersionInformation(it, end);
    if (it >= end) { return; }

    const auto nodesBlockSize = readUnsigned(it, end);
    if (it + nodesBlockSize > end) { return; }

    const auto nodesBlockEnd = it + nodesBlockSize;
    while(it < nodesBlockEnd) {
        way.nodes.push_back(readDelta(it, end, m_wayNodeIdDelta));
    }

    while (it < end) {
        readTagOrBbox(way, it, end);
    }
    std::sort(way.tags.begin(), way.tags.end());

   addWay(std::move(way));
}

void O5mParser::readRelation(const uint8_t *begin, const uint8_t *end)
{
    OSM::Relation rel;

    auto it = begin;
    rel.id = readDelta(it, end, m_relIdDelta);
    skipVersionInformation(it, end);
    if (it >= end) { return; }

    const auto relBlockSize = readUnsigned(it, end);
    if (it + relBlockSize > end) { return; }

    const auto relBlockEnd = it + relBlockSize;
    while (it < relBlockEnd) {
        const int64_t memId = readSigned(it, end);
        OSM::Member mem;
        const auto typeAndRole = readString(it, end);
        switch (typeAndRole[0]) {
            case O5M_MEMTYPE_NODE:
                mem.id = m_relNodeMemberIdDelta += memId;
                mem.setType(OSM::Type::Node);
                break;
            case O5M_MEMTYPE_WAY:
                mem.id = m_relWayMemberIdDelta += memId;
                mem.setType(OSM::Type::Way);
                break;
            case O5M_MEMTYPE_RELATION:
                mem.id = m_relRelMemberIdDelta += memId;
                mem.setType(OSM::Type::Relation);
                break;
        }
        mem.setRole(m_dataSet->makeRole(typeAndRole + 1, OSM::StringMemory::Transient));

        rel.members.push_back(std::move(mem));
    }



    while (it < end) {
        readTagOrBbox(rel, it, end);
    }
    std::sort(rel.tags.begin(), rel.tags.end());

    addRelation(std::move(rel));
}

void O5mParser::resetDeltaCodingState()
{
    m_nodeIdDelta = 0;
    m_latDelata = 0;
    m_lonDelta = 0;

    m_wayIdDelta = 0;
    m_wayNodeIdDelta = 0;

    m_relIdDelta = 0;
    m_relNodeMemberIdDelta = 0;
    m_relWayMemberIdDelta = 0;
    m_relRelMemberIdDelta = 0;
}
