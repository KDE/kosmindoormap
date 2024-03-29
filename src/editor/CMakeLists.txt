# SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-2-Clause

if (TARGET KF6::Service AND TARGET KF6::CoreAddons)
    set(HAVE_KSERVICE TRUE)
endif()
configure_file(config-editorcontroller.h.in ${CMAKE_CURRENT_BINARY_DIR}/config-editorcontroller.h)

if (ANDROID)
    add_library(KOSMEditorController)
else()
    add_library(KOSMEditorController STATIC)
endif()
target_sources(KOSMEditorController PRIVATE
    editorcontroller.cpp
)
generate_export_header(KOSMEditorController BASE_NAME KOSMEditorController)
set_target_properties(KOSMEditorController PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(KOSMEditorController PUBLIC KOSM PRIVATE Qt::Network Qt::Gui)
if (HAVE_KSERVICE)
    target_link_libraries(KOSMEditorController PRIVATE KF6::Service KF6::CoreAddons)
endif()

ecm_qt_declare_logging_category(KOSMEditorController
    HEADER logging.h
    IDENTIFIER KOSM::EditorLog
    CATEGORY_NAME org.kde.osm.editorcontroller
    DESCRIPTION "KOSM Editor Controller"
    EXPORT KOSMINDOORMAP
)

if (ANDROID)
    add_subdirectory(android)
    install(TARGETS KOSMEditorController ${KF_INSTALL_TARGETS_DEFAULT_ARGS})
endif()

ecm_add_qml_module(kosmeditorcontrollerplugin
    URI "org.kde.osm.editorcontroller"
    NO_GENERATE_PLUGIN_SOURCE
    NO_PLUGIN_OPTIONAL
    CLASS_NAME "KOSMEditorControllerPlugin"
)
target_sources(kosmeditorcontrollerplugin PRIVATE editorcontrollerplugin.cpp)
target_link_libraries(kosmeditorcontrollerplugin PRIVATE
    KOSMEditorController
    Qt::Quick
)
ecm_finalize_qml_module(kosmeditorcontrollerplugin)
