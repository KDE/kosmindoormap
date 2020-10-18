/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef OSM_XMLWRITER_H
#define OSM_XMLWRITER_H

#include <kosm_export.h>

class QIODevice;

namespace OSM {
class DataSet;

/** Serialite a OSM::DataSet into OSM XML. */
namespace KOSM_EXPORT XmlWriter
{
    void write(const OSM::DataSet &dataSet, QIODevice *out);
}

}

#endif // OSM_XMLWRITER_H
