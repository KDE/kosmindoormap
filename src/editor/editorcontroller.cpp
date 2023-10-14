/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "editorcontroller.h"
#include "logging.h"

#include <QDesktopServices>
#include <QUrl>
#include <QUrlQuery>

using namespace KOSM;

static void openElementInId(OSM::Element element)
{
    QUrl url;
    url.setScheme(QStringLiteral("https"));
    url.setHost(QStringLiteral("www.openstreetmap.org"));
    url.setPath(QStringLiteral("/edit"));

    QUrlQuery query;
    query.addQueryItem(QStringLiteral("editor"), QStringLiteral("id"));
    switch (element.type()) {
        case OSM::Type::Null:
            Q_UNREACHABLE();
        case OSM::Type::Node:
            query.addQueryItem(QStringLiteral("node"), QString::number(element.id()));
            break;
        case OSM::Type::Way:
            query.addQueryItem(QStringLiteral("way"), QString::number(element.id()));
            break;
        case OSM::Type::Relation:
            query.addQueryItem(QStringLiteral("relation"), QString::number(element.id()));
            break;
    }

    url.setQuery(query);
    QDesktopServices::openUrl(url);
}

void EditorController::editElement(OSM::Element element)
{
    if (element.type() == OSM::Type::Null) {
        return;
    }

    qCDebug(EditorLog) << element.url();
    openElementInId(element);
}
