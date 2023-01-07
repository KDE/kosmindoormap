# SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

add_definitions(-DSOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

ecm_add_test(osmtypetest.cpp LINK_LIBRARIES Qt::Test KOSM)
ecm_add_test(o5mparsertest.cpp LINK_LIBRARIES Qt::Test KOSM)

add_subdirectory(data/platforms)
ecm_add_test(mapviewtest.cpp LINK_LIBRARIES Qt::Test KOSMIndoorMap)
ecm_add_test(mapcssparsertest.cpp LINK_LIBRARIES Qt::Test KOSMIndoorMap)
ecm_add_test(scenegeometrytest.cpp LINK_LIBRARIES Qt::Test KOSMIndoorMap)
ecm_add_test(tilecachetest.cpp LINK_LIBRARIES Qt::Test KOSMIndoorMap)
ecm_add_test(marblegeometryassemblertest.cpp LINK_LIBRARIES Qt::Test KOSMIndoorMap)
ecm_add_test(mapleveltest.cpp LINK_LIBRARIES Qt::Test KOSMIndoorMap)
ecm_add_test(levelparsertest.cpp LINK_LIBRARIES Qt::Test KOSMIndoorMap)
ecm_add_test(penwidthutiltest.cpp LINK_LIBRARIES Qt::Test KOSMIndoorMap)
ecm_add_test(platformfindertest.cpp LINK_LIBRARIES Qt::Test KOSMIndoorMap KOSM_xmlioplugin)
ecm_add_test(platformmodeltest.cpp LINK_LIBRARIES Qt::Test KOSMIndoorMap KOSM_xmlioplugin)
ecm_add_test(osmelementinfomodeltest.cpp LINK_LIBRARIES Qt::Test KOSMIndoorMapQuick KOSM_xmlioplugin)
