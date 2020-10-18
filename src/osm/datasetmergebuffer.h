/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef OSM_DATASETMERGEBUFFER_H
#define OSM_DATASETMERGEBUFFER_H

#include <kosm_export.h>

#include "datatypes.h"

namespace OSM {

/** Holds OSM elements produced by a parser prior to merging into OSM::DataSet. */
class KOSM_EXPORT DataSetMergeBuffer
{
public:
    void clear();

    std::vector<OSM::Node> nodes;
    std::vector<OSM::Way> ways;
    std::vector<OSM::Relation> relations;
};

}

#endif
