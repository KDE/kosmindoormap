/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORROUTING_RECASTNAVDEBUG_P_H
#define KOSMINDOORROUTING_RECASTNAVDEBUG_P_H

#include <QIODevice>

#if HAVE_RECAST

#include <RecastDump.h>

namespace KOSMIndoorRouting {

struct RecastDebugIoAdapter : public duFileIO
{
public:
    explicit inline RecastDebugIoAdapter(QIODevice *io)
        : m_io(io)
    {}
    ~RecastDebugIoAdapter() override = default;

    [[nodiscard]] inline bool isWriting() const override
    {
        return m_io->openMode() & QIODevice::WriteOnly;
    }

    [[nodiscard]] inline bool isReading() const override
    {
        return m_io->openMode() & QIODevice::ReadOnly;
    }

    inline bool write(const void *ptr, const size_t size) override
    {
        return m_io->write(reinterpret_cast<const char*>(ptr), (qsizetype)size) == (qint64)size;
    }

    [[nodiscard]] inline bool read(void *ptr, const size_t size) override
    {
        return m_io->read(reinterpret_cast<char*>(ptr), (qsizetype)size);
    }

private:
    QIODevice *m_io = nullptr;
};

}

#endif

#endif
