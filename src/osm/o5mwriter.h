/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef OSM_O5MWRITER_H
#define OSM_O5MWRITER_H

#include "abstractwriter.h"

#include <cstdint>
#include <cstring>
#include <string>
#include <unordered_map>

#include <QDebug>

namespace OSM {

struct O5mStringPair {
    std::string s1;
    std::string s2;

    inline bool operator==(const O5mStringPair &other) const {
        return s1 == other.s1 && s2 == other.s2;
    }
};
}

template<>
struct std::hash<OSM::O5mStringPair>
{

    std::size_t operator()(const OSM::O5mStringPair &p) const noexcept
    {
        std::size_t h1 = std::hash<std::string>{}(p.s1);
        std::size_t h2 = std::hash<std::string>{}(p.s2);
        return h1 ^ (h2 << 1);
    }
};

namespace OSM {

/** Serialize an OSM::DataSet into the o5m file format. */
class O5mWriter : public OSM::AbstractWriter
{
private:
    void writeToIODevice(const OSM::DataSet& dataSet, QIODevice* io) override;

    void writeNodes(const OSM::DataSet &dataSet, QIODevice *io);
    void writeWays(const OSM::DataSet &dataSet, QIODevice *io);
    void writeRelations(const OSM::DataSet &dataSet, QIODevice *io);
    template <typename T> void writeTags(const T &elem, QIODevice *io);
    void writeStringPair(const char *s1, const char *s2, QIODevice *io);

    std::unordered_map<O5mStringPair, int16_t> m_stringTable;
};

}

#endif // OSM_O5MWRITER_H
