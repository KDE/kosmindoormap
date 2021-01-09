/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <kosmindoormap_version.h>

// #include <QQuickStyle>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#ifdef Q_OS_ANDROID
#include <QGuiApplication>
#else
#include <QApplication>
#endif

#include <QIcon>

#ifdef Q_OS_ANDROID
Q_DECL_EXPORT
#endif
int main(int argc, char **argv)
{
    QCoreApplication::setApplicationName(QStringLiteral("kosmindoormap"));
    QCoreApplication::setOrganizationName(QStringLiteral("KDE"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("kde.org"));
    QCoreApplication::setApplicationVersion(QStringLiteral(KOSMINDOORMAP_VERSION_STRING));

    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#ifdef Q_OS_ANDROID
    QGuiApplication app(argc, argv);
//     QQuickStyle::setStyle(QStringLiteral("Material"));
#else
    QApplication app(argc, argv); // for native file dialogs
#endif
    QGuiApplication::setApplicationDisplayName(QStringLiteral("KDE OSM Indoor Map"));
    QGuiApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral("go-home")));

    QQmlApplicationEngine engine;
    engine.load(QStringLiteral("qrc:/indoormap.qml"));
    return app.exec();
}
