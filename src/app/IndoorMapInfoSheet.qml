/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kosmindoormap
import org.kde.osm.editorcontroller

OSMElementInformationDialog {
    id: elementDetailsSheet
    customFooterActions: [
        Kirigami.Action {
            icon.name: "document-edit"
            text: "Edit with iD"
            visible: elementDetailsSheet.model.element.id > 0
            onTriggered: EditorController.editElement(elementDetailsSheet.model.element.element, Editor.ID)
        },
        Kirigami.Action {
            icon.name: "org.openstreetmap.josm"
            text: "Edit with JOSM"
            visible: elementDetailsSheet.model.element.id > 0 && EditorController.hasEditor(Editor.JOSM)
            onTriggered: EditorController.editElement(elementDetailsSheet.model.element.element, Editor.JOSM)
        },
        Kirigami.Action {
            icon.name: "document-edit"
            text: "Edit with Vespucci"
            visible: elementDetailsSheet.model.element.id > 0 && EditorController.hasEditor(Editor.Vespucci)
            onTriggered: EditorController.editElement(elementDetailsSheet.model.element.element, Editor.Vespucci)
        },
        Kirigami.Action {
            property string wikidataId: elementDetailsSheet.model.element.tagValue(["wikidata", "brand:wikidata", "species:wikidata", "subject:wikidata"])
            icon.name: "document-edit"
            text: "Edit Wikidata"
            visible: wikidataId.match(/^Q\d+$/)
            onTriggered: Qt.openUrlExternally("https://wikidata.org/wiki/" + wikidataId)
        }
    ]
}
