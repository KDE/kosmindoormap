# SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-2-Clause

add_library(KOSMEditorController STATIC
    editorcontroller.cpp
)
set_target_properties(KOSMEditorController PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(KOSMEditorController PUBLIC KOSM PRIVATE Qt::Network Qt::Gui)

ecm_qt_declare_logging_category(KOSMEditorController
    HEADER logging.h
    IDENTIFIER KOSM::EditorLog
    CATEGORY_NAME org.kde.osm.editorcontroller
    DESCRIPTION "KOSM Editor Controller"
    EXPORT KOSMINDOORMAP
)

add_library(kosmeditorcontrollerplugin editorcontrollerplugin.cpp)
target_link_libraries(kosmeditorcontrollerplugin
    KOSMEditorController
    Qt::Quick
)

install(TARGETS kosmeditorcontrollerplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/osm/editorcontroller)
install(FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/osm/editorcontroller)
