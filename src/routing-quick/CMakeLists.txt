# SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-2-Clause

ecm_add_qml_module(kosmindoorroutingquickplugin
    URI "org.kde.kosmindoorrouting"
    DEPENDENCIES
        "org.kde.kosmindoormap"
    GENERATE_PLUGIN_SOURCE
)
target_sources(kosmindoorroutingquickplugin PRIVATE
    routingarea.h
    routingcontroller.cpp
)
if (TARGET RecastNavigation::Recast AND TARGET RecastNavigation::Detour)
    target_compile_definitions(kosmindoorroutingquickplugin PRIVATE HAVE_RECAST=1)
else()
    target_compile_definitions(kosmindoorroutingquickplugin PRIVATE HAVE_RECAST=0)
endif()
target_link_libraries(kosmindoorroutingquickplugin PRIVATE
    KOSMIndoorRouting
)

ecm_finalize_qml_module(kosmindoorroutingquickplugin)
