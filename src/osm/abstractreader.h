/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef OSM_ABSTRACTREADER_H
#define OSM_ABSTRACTREADER_H

#include "kosm_export.h"

namespace OSM {

class DataSet;
class DataSetMergeBuffer;

/** Abstract base class for OSM file format readers. */
class KOSM_EXPORT AbstractReader
{
public:
    virtual ~AbstractReader();

    /** Sets a merge buffer.
     *  When set, the parser will insert all elements into that buffer
     *  rather than in the OSM::DataSet specified in the constructor.
     *  It is then your responsibility to properly integrate those.
     *  @note The OSM::DataSet is used for generating tag keys and for memory
     *  managing strings in this case as well. So the generated elements are
     *  tied to the OSM::DataSet in any case.
     */
    void setMergeBuffer(OSM::DataSetMergeBuffer *buffer);

protected:
    explicit AbstractReader(DataSet *dataSet);

    DataSet *m_dataSet = nullptr;
    DataSetMergeBuffer *m_mergeBuffer = nullptr;
};

}

#endif // OSM_ABSTRACTREADER_H
