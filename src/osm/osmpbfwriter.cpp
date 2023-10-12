/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "osmpbfwriter.h"
#include "datatypes.h"

#include "fileformat.pb.h"
#include "osmformat.pb.h"

#include <QIODevice>
#include <QtEndian>

#include <zlib.h>

using namespace OSM;

static constexpr const std::size_t BLOCK_SIZE_LIMIT = 16'777'216;

OsmPbfWriter::OsmPbfWriter() = default;
OsmPbfWriter::~OsmPbfWriter() = default;

void OsmPbfWriter::writeToIODevice(const OSM::DataSet &dataSet, QIODevice *io)
{
    m_io = io;
    writeNodes(dataSet);
    writeWays(dataSet);
    writeRelations(dataSet);
    if (m_block) {
        writeBlob();
    }
    m_io =  nullptr;
}

void OsmPbfWriter::writeNodes(const OSM::DataSet &dataSet)
{
    if (dataSet.nodes.empty()) {
        return;
    }

    int64_t prevId = 0;
    int64_t prevLat = 900'000'000ll;
    int64_t prevLon = 1'800'000'000ll;

    OSMPBF::DenseNodes *denseBlock = nullptr;
    for(auto const &node: dataSet.nodes) {
        createBlockIfNeeded();
        if (!denseBlock) {
            denseBlock = m_block->add_primitivegroup()->mutable_dense();
        }

        denseBlock->add_id(node.id - prevId);
        prevId = node.id;

        denseBlock->add_lat((int64_t)node.coordinate.latitude - prevLat);
        prevLat = node.coordinate.latitude;
        denseBlock->add_lon((int64_t)node.coordinate.longitude - prevLon);
        prevLon = node.coordinate.longitude;

        for (const auto &tag : node.tags) {
            denseBlock->add_keys_vals(stringTableEntry(tag.key.name()));
            denseBlock->add_keys_vals(stringTableEntry(tag.value.constData()));
            m_blockSizeEstimate += 2 * sizeof(int32_t);
        }

        denseBlock->add_keys_vals(0);
        m_blockSizeEstimate += 3 * sizeof(int64_t) + sizeof(int32_t);

        if (blockSizeLimitReached()) {
            denseBlock = nullptr;
            writeBlob();
        }
    }
}

void OsmPbfWriter::writeWays(const OSM::DataSet &dataSet)
{
    OSMPBF::PrimitiveGroup *group = nullptr;
    for(auto const &way : dataSet.ways) {
        createBlockIfNeeded();
        if (!group) {
            group = m_block->add_primitivegroup();
        }

        auto w = group->add_ways();
        w->set_id(way.id);
        m_blockSizeEstimate += sizeof(int64_t);

        int64_t prevId = 0;
        for (const auto &id : way.nodes) {
            w->add_refs(id - prevId);
            prevId = id;
            m_blockSizeEstimate += sizeof(int64_t);
        }
        for (const auto &tag : way.tags) {
            w->add_keys(stringTableEntry(tag.key.name()));
            w->add_vals(stringTableEntry(tag.value.constData()));
            m_blockSizeEstimate += 2 * sizeof(int32_t);
        }

        if (blockSizeLimitReached()) {
            group = nullptr;
            writeBlob();
        }
    }
}

static OSMPBF::Relation_MemberType pbfMemberType(OSM::Type t)
{
    switch (t) {
        case OSM::Type::Null:
            Q_UNREACHABLE();
        case OSM::Type::Node:
            return OSMPBF::Relation_MemberType::Relation_MemberType_NODE;
        case OSM::Type::Way:
            return OSMPBF::Relation_MemberType::Relation_MemberType_WAY;
        case OSM::Type::Relation:
            return OSMPBF::Relation_MemberType::Relation_MemberType_RELATION;
    }
    return OSMPBF::Relation_MemberType::Relation_MemberType_NODE;
}

void OsmPbfWriter::writeRelations(const OSM::DataSet &dataSet)
{
    OSMPBF::PrimitiveGroup *group = nullptr;
    for (const auto &rel :dataSet.relations) {
        createBlockIfNeeded();
        if (!group) {
            group = m_block->add_primitivegroup();
        }

        auto r = group->add_relations();
        r->set_id(rel.id);
        m_blockSizeEstimate += sizeof(int64_t);

        for (const auto &tag : rel.tags) {
            r->add_keys(stringTableEntry(tag.key.name()));
            r->add_vals(stringTableEntry(tag.value.constData()));
            m_blockSizeEstimate += 2 * sizeof(int32_t);
        }

        int64_t prevMemId = 0;
        for (const auto &mem : rel.members) {
            r->add_roles_sid(stringTableEntry(mem.role().name()));
            r->add_memids(mem.id - prevMemId);
            prevMemId = mem.id;
            r->add_types(pbfMemberType(mem.type()));
            m_blockSizeEstimate += 2* sizeof(int32_t) + sizeof(int64_t);
        }

        if (blockSizeLimitReached()) {
            group = nullptr;
            writeBlob();
        }
    }
}

int32_t OsmPbfWriter::stringTableEntry(const char *s)
{
    assert(m_block);
    const auto it = m_stringTable.find(s);
    if (it == m_stringTable.end()) {
        auto st = m_block->mutable_stringtable();
        st->add_s(s);
        m_stringTable[s] = st->s_size() - 1;
        m_blockSizeEstimate += std::strlen(s) + 1 + sizeof(int32_t);
        return st->s_size() - 1;
    }

    return (*it).second;
}

void OsmPbfWriter::createBlockIfNeeded()
{
    if (!m_block) {
        m_block = std::make_unique<OSMPBF::PrimitiveBlock>();
        m_blockSizeEstimate = 0;
        m_block->mutable_stringtable()->add_s(""); // dense node block tag separation marker
    }
}

bool OsmPbfWriter::blockSizeLimitReached() const
{
    return m_blockSizeEstimate >BLOCK_SIZE_LIMIT;
}

void OsmPbfWriter::writeBlob()
{
    OSMPBF::Blob blob;
    auto rawBlobData = m_block->SerializeAsString();
    auto zlibBlobData = blob.mutable_zlib_data();
    zlibBlobData->resize(32 * 1024ul * 1024ul);

    z_stream zStream;
    zStream.next_in = (uint8_t*)rawBlobData.data();
    zStream.avail_in = rawBlobData.size();
    zStream.next_out = (uint8_t*)zlibBlobData->data();
    zStream.avail_out = zlibBlobData->size();
    zStream.zalloc = nullptr;
    zStream.zfree = nullptr;
    zStream.opaque = nullptr;
    deflateInit(&zStream, Z_DEFAULT_COMPRESSION);
    while (true) {
        const auto ret = deflate(&zStream, Z_FINISH);
        if (ret == Z_STREAM_END) {
            break;
        }
        if (ret != Z_OK) {
            qWarning() << "zlib compression error!" << ret;
            break;
        }
        if (zStream.avail_out == 0) {
            // we could dynamically resize the output buffer here, but that
            // is already about twice the size of the data we want to compress
            // so we really shouldn't end up here
            qFatal("zlib output buffer underrun");
            break;
        }
    }
    zlibBlobData->resize(zlibBlobData->size() - zStream.avail_out);

    deflateEnd(&zStream);
    blob.set_raw_size((int32_t)rawBlobData.size());

    OSMPBF::BlobHeader header;
    header.set_type("OSMData");
    header.set_datasize((int32_t)blob.ByteSizeLong());

    auto blobHeaderSize = (int32_t)header.ByteSizeLong();
    blobHeaderSize = qToBigEndian(blobHeaderSize);
    m_io->write(reinterpret_cast<const char*>(&blobHeaderSize), sizeof(blobHeaderSize));
    m_io->write(header.SerializeAsString().c_str(), header.ByteSizeLong()); // TODO do this copy-free
    m_io->write(blob.SerializeAsString().c_str(), blob.ByteSizeLong()); // TODO do this copy-free

    m_block.reset();
    m_blockSizeEstimate = 0;
    m_stringTable.clear();
}
