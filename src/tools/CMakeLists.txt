# SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

add_executable(osm-download-data osm-download-data.cpp)
target_link_libraries(osm-download-data KOSMIndoorMap KOSM_xmlioplugin)
