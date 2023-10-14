/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "editorcontroller.h"

#include <QJSEngine>
#include <QQmlEngine>
#include <QQmlExtensionPlugin>

class KOSMEditorControllerPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")
public:
    void registerTypes(const char *uri) override
    {
        Q_UNUSED(uri);
        qRegisterMetaType<OSM::Element>();
        qmlRegisterSingletonType("org.kde.osm.editorcontroller", 1, 0, "EditorController", [](QQmlEngine*, QJSEngine *engine) -> QJSValue {
            return engine->toScriptValue(KOSM::EditorController());
        });
        qmlRegisterUncreatableMetaObject(KOSM::EditorController::staticMetaObject, "org.kde.osm.editorcontroller", 1, 0, "Editor", {});
    }
};

#include "editorcontrollerplugin.moc"
