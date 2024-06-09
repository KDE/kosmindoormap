# SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-2-Clause

add_library(KWikidata STATIC
    entities.cpp
    wikidataquerymanager.cpp
    wikidataquery.cpp
    entities.h
    wikidataquerymanager.h
    wikidataquery.h
)

target_include_directories(KOSM PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>")
target_link_libraries(KWikidata
    PUBLIC Qt::Gui
    PRIVATE Qt::Network
)
set_target_properties(KWikidata PROPERTIES POSITION_INDEPENDENT_CODE ON)

ecm_qt_declare_logging_category(KWikidata
    HEADER logging.h
    IDENTIFIER Wikidata::Log
    CATEGORY_NAME org.kde.wikidata
    DESCRIPTION "KWikidata"
    EXPORT KWIKIDATA
)
