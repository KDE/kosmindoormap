/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORMAP_MAPCSSLOADER_H
#define KOSMINDOORMAP_MAPCSSLOADER_H

#include "kosmindoormap_export.h"

#include <KOSMIndoorMap/NetworkAccessManagerFactory>

#include <QObject>
#include <QUrl>

#include <memory>

namespace KOSMIndoorMap {

class MapCSSLoaderPrivate;
class MapCSSStyle;

/** Asynchronous loader for (remote) MapCSS assets. */
class KOSMINDOORMAP_EXPORT MapCSSLoader : public QObject {
    Q_OBJECT
public:
    /** Create MapCSS loading/parsing job for @p style.
     *  This will download online assets if necessary.
     *  @param style Resolved style URL, can be local file, qrc or HTTP.
     *  @see start(), finished()
     */
    explicit MapCSSLoader(const QUrl &style, const NetworkAccessManagerFactory &nam, QObject *parent = nullptr);
    ~MapCSSLoader();

    /** Start loading. */
    void start();

    /** The fully loaded and parsed style.
     *  Only valid when hasError() returns @c false and finished() has been emitted.
     */
    [[nodiscard]] MapCSSStyle&& takeStyle();

    /** Check whether loading or parsing failed in some way. */
    [[nodiscard]] bool hasError() const;
    [[nodiscard]] QString errorMessage() const;

    /** Resolve @p style to an absolute URL to load.
     *  @param baseUrl the location to use for resolving relative imports.
     */
    [[nodiscard]] static QUrl resolve(const QString &style, const QUrl &baseUrl = {});

    /** Translate local or remote URL to locally loadable (cache) file. */
    [[nodiscard]] static QString toLocalFile(const QUrl &url);

    /** Expire locally cached remote MapCSS assets. */
    static void expire();

Q_SIGNALS:
    /** Loading is done, successfully or with an error. */
    void finished();

private:
    Q_DECL_HIDDEN void download(const QUrl &url);
    std::unique_ptr<MapCSSLoaderPrivate> d;
};

}

#endif
