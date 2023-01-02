/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef OSM_READER_H
#define OSM_READER_H

#include "kosm_export.h"
#include "abstractreader.h"

class QStringView;

#include <memory>

namespace OSM {

class DataSet;

/** Access to OSM file/data readers. */
namespace Reader
{

/** Returns a suitable reader for the given file name. */
KOSM_EXPORT std::unique_ptr<AbstractReader> readerForFileName(QStringView fileName, OSM::DataSet *dataSet);

}

}

#endif // OSM_READER_H
