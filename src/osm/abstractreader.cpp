/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "abstractreader.h"

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
