/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "abstractreader.h"
#include "datatypes.h"
#include "datasetmergebuffer.h"

#include <cassert>

using namespace OSM;

AbstractReader::AbstractReader(OSM::DataSet *dataSet)
    : m_dataSet(dataSet)
{
    assert(dataSet);
}

AbstractReader::~AbstractReader() = default;

void AbstractReader::setMergeBuffer(OSM::DataSetMergeBuffer *buffer)
{
    m_mergeBuffer = buffer;
}

void AbstractReader::addNode(OSM::Node &&node)
{
    m_mergeBuffer ? m_mergeBuffer->nodes.push_back(std::move(node)) : m_dataSet->addNode(std::move(node));
}

void AbstractReader::addWay(OSM::Way &&way)
{
    m_mergeBuffer ? m_mergeBuffer->ways.push_back(std::move(way)) : m_dataSet->addWay(std::move(way));
}

void AbstractReader::addRelation(OSM::Relation &&relation)
{
    m_mergeBuffer ? m_mergeBuffer->relations.push_back(std::move(relation)) : m_dataSet->addRelation(std::move(relation));
}
