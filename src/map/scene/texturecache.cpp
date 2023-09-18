/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "texturecache_p.h"
#include "logging.h"

#include <QDebug>
#include <QFile>
#include <QGuiApplication>
#include <QImageReader>

using namespace KOSMIndoorMap;

TextureCache::TextureCache() = default;
TextureCache::~TextureCache() = default;

QImage TextureCache::image(const QString &name) const
{
    auto it = std::lower_bound(m_cache.begin(), m_cache.end(), name, [](const auto &lhs, const auto &rhs) { return lhs.name < rhs; });
    if (it != m_cache.end() && (*it).name == name) {
        return (*it).image;
    }

    CacheEntry entry;
    entry.name = name;

    const QString fileName = QLatin1String(":/org.kde.kosmindoormap/assets/textures/") + name;
    if (name.endsWith(QLatin1String(".svg"))) {
        QImageReader imgReader(fileName, "svg");
        imgReader.setScaledSize(imgReader.size() * qGuiApp->devicePixelRatio());
        entry.image = imgReader.read();
        entry.image.setDevicePixelRatio(qGuiApp->devicePixelRatio());
    } else {
        // TODO high dpi raster image loading
        // QImageReader is supposed to do that transparently, but that doesn't seem to work here?
        QImageReader imgReader(fileName);
        entry.image = imgReader.read();
    }

    if (entry.image.isNull()) {
        qCWarning(Log) << "failed to load texture:" << name;
    } else {
        qCDebug(Log) << "loaded texture:" << entry.name << entry.image;
    }

    it = m_cache.insert(it, std::move(entry));
    return (*it).image;
}
