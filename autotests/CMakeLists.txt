# SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

add_definitions(-DSOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

ecm_add_test(osmtypetest.cpp LINK_LIBRARIES Qt::Test KOSM)
ecm_add_test(o5mparsertest.cpp LINK_LIBRARIES Qt::Test KOSM)
ecm_add_test(oscparsertest.cpp LINK_LIBRARIES Qt::Test KOSM)
ecm_add_test(localizedtagtest.cpp LINK_LIBRARIES Qt::Test KOSM)

add_subdirectory(data/platforms)
ecm_add_test(mapviewtest.cpp LINK_LIBRARIES Qt::Test KOSMIndoorMap)
ecm_add_test(mapcssparsertest.cpp LINK_LIBRARIES Qt::Test KOSMIndoorMap)
ecm_add_test(mapcssexpressiontest.cpp LINK_LIBRARIES Qt::Test KOSMIndoorMap)
ecm_add_test(mapcssloadertest.cpp LINK_LIBRARIES Qt::Test KOSMIndoorMap)
ecm_add_test(scenegeometrytest.cpp LINK_LIBRARIES Qt::Test KOSMIndoorMap)
ecm_add_test(tilecachetest.cpp LINK_LIBRARIES Qt::Test KOSMIndoorMap)
ecm_add_test(marblegeometryassemblertest.cpp LINK_LIBRARIES Qt::Test KOSMIndoorMap)
ecm_add_test(mapleveltest.cpp LINK_LIBRARIES Qt::Test KOSMIndoorMap)
ecm_add_test(levelparsertest.cpp LINK_LIBRARIES Qt::Test KOSMIndoorMap)
ecm_add_test(penwidthutiltest.cpp LINK_LIBRARIES Qt::Test KOSMIndoorMap)
ecm_add_test(platformfindertest.cpp LINK_LIBRARIES Qt::Test KOSMIndoorMap)
ecm_add_test(platformmodeltest.cpp LINK_LIBRARIES Qt::Test KOSMIndoorMap)
ecm_add_test(osmelementinfomodeltest.cpp LINK_LIBRARIES Qt::Test KOSMIndoorMapQuick)
ecm_add_test(amenitymodeltest.cpp LINK_LIBRARIES Qt::Test KOSMIndoorMapQuick)
ecm_add_test(openinghourscachetest.cpp LINK_LIBRARIES Qt::Test KOSMIndoorMapQuick KOpeningHours)
ecm_add_test(osmconditionalexpressiontest.cpp LINK_LIBRARIES Qt::Test KOSMIndoorMapQuick KOpeningHours)

# verify QML code
if (TARGET kosmindoormap-app)
    add_test(NAME kosmindoormap-self-test COMMAND kosmindoormap-app --self-test)
endif()
