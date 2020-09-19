# SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

add_definitions(-DSOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

ecm_add_test(osmtypetest.cpp LINK_LIBRARIES Qt5::Test KOSM)
ecm_add_test(o5mparsertest.cpp LINK_LIBRARIES Qt5::Test KOSM)

if (TARGET KOSMIndoorMap)
    ecm_add_test(mapviewtest.cpp LINK_LIBRARIES Qt5::Test KOSMIndoorMap)
    ecm_add_test(mapcssparsertest.cpp LINK_LIBRARIES Qt5::Test KOSMIndoorMap)
    ecm_add_test(scenegeometrytest.cpp LINK_LIBRARIES Qt5::Test KOSMIndoorMap)
    ecm_add_test(tilecachetest.cpp LINK_LIBRARIES Qt5::Test KOSMIndoorMap)
    ecm_add_test(marblegeometryassemblertest.cpp LINK_LIBRARIES Qt5::Test KOSMIndoorMap)
    ecm_add_test(penwidthutiltest.cpp LINK_LIBRARIES Qt5::Test KOSMIndoorMap)
    ecm_add_test(osmelementinfomodeltest.cpp LINK_LIBRARIES Qt5::Test KOSMIndoorMapQuick)
endif()
