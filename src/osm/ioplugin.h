/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef OSM_IOPLUGIN_H
#define OSM_IOPLUGIN_H

#include "kosm_export.h"

#include <QtPlugin>

#include <memory>

namespace OSM {

class AbstractReader;
class DataSet;

/** Plugin interface for OSM file/data readers. */
class KOSM_EXPORT IOPluginInterface
{
public:
    virtual ~IOPluginInterface();

    /** Create a new reader instance. */
    virtual std::unique_ptr<AbstractReader> createReader(OSM::DataSet *dataSet) = 0;
};

template <typename T>
class IOPlugin : public IOPluginInterface
{
public:
    inline std::unique_ptr<AbstractReader> createReader(OSM::DataSet *dataSet) override
    {
        return std::make_unique<T>(dataSet);
    }
};

}

#define OSMIOPluginInteraface_iid "org.kde.kosm.IOPluginInterface/1.0"
Q_DECLARE_INTERFACE(OSM::IOPluginInterface, OSMIOPluginInteraface_iid)

#endif // OSM_IOPLUGIN_H

