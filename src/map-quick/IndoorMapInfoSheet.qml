/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import org.kde.kirigami as Kirigami
import org.kde.kosmindoormap
import org.kde.osm.editorcontroller

/*!
    \qmltype IndoorMapInfoSheet
    \inqmlmodule org.kde.kosmindoormap
    \inherits OSMElementInformationDialog
    \brief Displays information about an OSM element with optional editing actions.
*/
OSMElementInformationDialog {
    id: elementDetailsSheet

    /*!
        \qmlproperty bool IndoorMapInfoSheet::allowEditing

        Whether to show actions for editing OSM and Wikidata elements.
        The default is \c true.
    */
    property bool allowEditing: true

    customFooterActions: [
        Kirigami.Action {
            icon.name: "document-edit"
            text: i18nd("kosmindoormap", "Edit with iD")
            visible: elementDetailsSheet.allowEditing && elementDetailsSheet.model.element.id > 0
            onTriggered: EditorController.editElement(elementDetailsSheet.model.element.element, Editor.ID)
        },
        Kirigami.Action {
            icon.name: "org.openstreetmap.josm"
            text: i18nd("kosmindoormap", "Edit with JOSM")
            visible: elementDetailsSheet.allowEditing && elementDetailsSheet.model.element.id > 0 && EditorController.hasEditor(Editor.JOSM)
            onTriggered: EditorController.editElement(elementDetailsSheet.model.element.element, Editor.JOSM)
        },
        Kirigami.Action {
            icon.name: "document-edit"
            text: i18nd("kosmindoormap", "Edit with Vespucci")
            visible: elementDetailsSheet.allowEditing && elementDetailsSheet.model.element.id > 0 && EditorController.hasEditor(Editor.Vespucci)
            onTriggered: EditorController.editElement(elementDetailsSheet.model.element.element, Editor.Vespucci)
        },
        Kirigami.Action {
            readonly property string wikidataId: elementDetailsSheet.model.element.tagValue(["wikidata", "brand:wikidata", "species:wikidata", "subject:wikidata"])
            icon.name: "document-edit"
            text: i18nd("kosmindoormap", "Edit Wikidata")
            visible: elementDetailsSheet.allowEditing && wikidataId.match(/^Q\d+$/)
            onTriggered: Qt.openUrlExternally("https://wikidata.org/wiki/" + wikidataId)
        }
    ]
}
