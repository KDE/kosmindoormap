/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef OSM_OSMPBFWRITER_H
#define OSM_OSMPBFWRITER_H

#include "abstractwriter.h"

#include <cstdint>
#include <cstring>
#include <string>

#include <QDebug>

namespace OSMPBF {
class PrimitiveBlock;
}

namespace OSM {

/** Serialize an OSM::DataSet into the PBF file format. */
class OsmPbfWriter : public OSM::AbstractWriter
{
public:
    explicit OsmPbfWriter();
    ~OsmPbfWriter() override;
private:
    void writeToIODevice(const OSM::DataSet &dataSet, QIODevice *io) override;

    void writeNodes(const OSM::DataSet &dataSet);
    void writeWays(const OSM::DataSet &dataSet);
    void writeRelations(const OSM::DataSet &dataSet);

    [[nodiscard]] int32_t stringTableEntry(const char *s);
    void createBlockIfNeeded();
    [[nodiscard]] bool blockSizeLimitReached() const;
    void writeBlob();

    std::unique_ptr<OSMPBF::PrimitiveBlock> m_block;
    std::map<std::string_view, int32_t> m_stringTable;
    std::size_t m_blockSizeEstimate = 0;
    QIODevice *m_io = nullptr;
};

}

#endif
