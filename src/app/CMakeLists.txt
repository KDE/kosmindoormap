# SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

add_executable(kosmindoormap-app
    main.cpp
    qml.qrc
)

target_include_directories(kosmindoormap-app PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(kosmindoormap-app PRIVATE
    KOSMIndoorMap
    Qt5::Quick
    #Qt5::QuickControls2
)

if (ANDROID)
    #target_include_directories(itinerary-app PRIVATE ${Qt5Core_PRIVATE_INCLUDE_DIRS})
    # explicitly add runtime dependencies and transitive link dependencies,
    # so androiddeployqt picks them up
    #target_link_libraries(kosmindoormap-app PRIVATE
        #KF5::Kirigami2
        #Qt5::Svg
        #OpenSSL::SSL
    #)

    #kirigami_package_breeze_icons(ICONS
        #document-open
    #)
else ()
    target_link_libraries(kosmindoormap-app PRIVATE
        Qt5::Widgets
    )
    set_target_properties(kosmindoormap-app PROPERTIES OUTPUT_NAME "kosmindoormap")
endif()

#install(TARGETS kosmindoormap-app ${INSTALL_TARGETS_DEFAULT_ARGS})
#install(PROGRAMS org.kde.kosmindoormap.desktop DESTINATION ${KDE_INSTALL_APPDIR})
#install(FILES org.kde.kosmindoormap.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
