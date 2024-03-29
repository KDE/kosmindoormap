/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef OSM_O5MPARSER_H
#define OSM_O5MPARSER_H

#include "kosm_export.h"
#include "abstractreader.h"

#include <cstddef>
#include <cstdint>
#include <vector>

class O5mParserTest;

namespace OSM {

class DataSet;
class DataSetMergeBuffer;

/** Zero-copy parser of O5M binary files.
 *  @see https://wiki.openstreetmap.org/wiki/O5m
 */
class KOSM_EXPORT O5mParser : public AbstractReader
{
public:
    explicit O5mParser(DataSet *dataSet);

private:
    void readFromData(const uint8_t *data, std::size_t len) override;

    friend class ::O5mParserTest;

    [[nodiscard]] uint64_t readUnsigned(const uint8_t *&it, const uint8_t *endIt) const;
    [[nodiscard]] int64_t readSigned(const uint8_t *&it, const uint8_t *endIt) const;
    template <typename T>
    [[nodiscard]] T readDelta(const uint8_t *&it, const uint8_t *endIt, T &deltaState);

    [[nodiscard]] const char* readString(const uint8_t *&it, const uint8_t *endIt);
    [[nodiscard]] std::pair<const char*, const char*> readStringPair(const uint8_t *&it, const uint8_t *endIt);

    void skipVersionInformation(const uint8_t *&it, const uint8_t *end);
    template <typename Elem>
    void readTagOrBbox(Elem &e, const uint8_t *&it, const uint8_t *endIt);

    void readNode(const uint8_t *begin, const uint8_t *end);
    void readWay(const uint8_t *begin, const uint8_t *end);
    void readRelation(const uint8_t *begin, const uint8_t *end);

    // delta coding state
    void resetDeltaCodingState();

    int64_t m_nodeIdDelta = 0;
    int32_t m_latDelata = 0; // this can overflow, but that is intentional according to the spec!
    int32_t m_lonDelta = 0;

    int64_t m_wayIdDelta = 0;
    int64_t m_wayNodeIdDelta = 0;

    int64_t m_relIdDelta = 0;
    int64_t m_relNodeMemberIdDelta = 0;
    int64_t m_relWayMemberIdDelta = 0;
    int64_t m_relRelMemberIdDelta = 0;

    std::vector<const char*> m_stringLookupTable;
    uint16_t m_stringLookupPosition = 0;
};

}

#endif // OSM_O5MPARSER_H
