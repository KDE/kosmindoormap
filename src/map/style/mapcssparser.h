/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORMAP_MAPCSSPARSER_H
#define KOSMINDOORMAP_MAPCSSPARSER_H

#include "kosmindoormap_export.h"

#include <memory>

class QString;
class QUrl;

namespace KOSMIndoorMap {

class MapCSSParserPrivate;
class MapCSSStyle;
class MapCSSRule;

/** MapCSS parser. */
class KOSMINDOORMAP_EXPORT MapCSSParser
{
public:
    explicit MapCSSParser();
    ~MapCSSParser();

    [[nodiscard]] MapCSSStyle parse(const QString &fileName);
    /** Parse MapCSS style sheet at @p url.
     *  This can be a local file, qrc resource or a HTTP URL.
     *  @note Attempting to parse a HTTP URL that isn't already downloaded will fail
     *  with FileNotFoundError. When using remote MapCSS files, don't use this directly
     *  but use MapCSSLoader which will handle necesary downloads itself.
     *  @see MapCSSLoader.
     */
    [[nodiscard]] MapCSSStyle parse(const QUrl &url);

    /** Returns @c true if an error occured during parsing and the returned style
     *  is invalid.
     */
    [[nodiscard]] bool hasError() const;

    enum Error {
        NoError,
        SyntaxError,
        FileNotFoundError,
        FileIOError,
        NetworkError,
    };
    [[nodiscard]] Error error() const;

    /** URL of the parsed MapCSS style sheet.
     *  This can be a local file, QRC asset or HTTP remote content.
     */
    [[nodiscard]] QUrl url() const;
    [[nodiscard]] QString errorMessage() const;

private:
    friend class MapCSSParserPrivate;
    std::unique_ptr<MapCSSParserPrivate> d;
};

}

#endif // KOSMINDOORMAP_MAPCSSPARSER_H
