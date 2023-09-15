/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "texturecache_p.h"
#include "logging.h"

#include <QDebug>
#include <QFile>

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
    // TODO high-dpi support
    entry.image = QImage(QLatin1String(":/org.kde.kosmindoormap/assets/textures/") + name);
    if (entry.image.isNull()) {
        qCWarning(Log) << "failed to load texture:" << name;
    } else {
        qCDebug(Log) << "loaded texture:" << entry.name << entry.image;
    }

    it = m_cache.insert(it, std::move(entry));
    return (*it).image;
}
