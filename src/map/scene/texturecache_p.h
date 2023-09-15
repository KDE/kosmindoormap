/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORMAP_TEXTURECACHE_P_H
#define KOSMINDOORMAP_TEXTURECACHE_P_H

#include <QImage>
#include <QString>

#include <vector>

namespace KOSMIndoorMap {

/** Texture loader and texture caching for images referenced in MapCSS. */
class TextureCache
{
public:
    explicit TextureCache();
    ~TextureCache();

    QImage image(const QString &name) const;

private:
    struct CacheEntry {
        QString name;
        QImage image;
    };
    mutable std::vector<CacheEntry> m_cache;
};

}

#endif // KOSMINDOORMAP_TEXTURECACHE_P_H
