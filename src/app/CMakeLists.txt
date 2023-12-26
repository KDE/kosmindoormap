# SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

add_executable(kosmindoormap-app
    main.cpp
    qml.qrc
)

target_include_directories(kosmindoormap-app PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(kosmindoormap-app PRIVATE
    KOSMIndoorMap
    KOSM_xmlioplugin
    KF6::I18n
    Qt::Quick
)

if (TARGET KOSM_pbfioplugin)
    target_compile_definitions(kosmindoormap-app PRIVATE -DHAVE_OSM_PBF_SUPPORT=1)
    target_link_libraries(kosmindoormap-app PRIVATE KOSM_pbfioplugin)
else()
    target_compile_definitions(kosmindoormap-app PRIVATE -DHAVE_OSM_PBF_SUPPORT=0)
endif()

if (ANDROID)
    # explicitly add runtime dependencies and transitive link dependencies,
    # so androiddeployqt picks them up
    target_link_libraries(kosmindoormap-app PRIVATE
        KF6::Kirigami2
        Qt::Svg
        OpenSSL::SSL
        Qt::QuickControls2
    )

    kirigami_package_breeze_icons(ICONS
        arrow-right
        document-open-symbolic
        edit-clear-symbolic
        edit-paste-symbolic
        go-down-symbolic
        go-home
        go-up-symbolic
        help-about-symbolic
        license
        search
        view-refresh-symbolic
    )

    ecm_add_android_apk(kosmindoormap-app ANDROID_DIR ${CMAKE_CURRENT_SOURCE_DIR}/android)
else ()
    target_link_libraries(kosmindoormap-app PRIVATE
        Qt::Widgets
    )
    set_target_properties(kosmindoormap-app PROPERTIES OUTPUT_NAME "kosmindoormap")
endif()

if (BUILD_STANDALONE_APP)
    install(TARGETS kosmindoormap-app ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
    if (NOT ANDROID)
        install(PROGRAMS org.kde.kosmindoormap.desktop DESTINATION ${KDE_INSTALL_APPDIR})
        #install(FILES org.kde.kosmindoormap.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
    endif()
endif()
