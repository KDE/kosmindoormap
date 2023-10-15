# SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-2-Clause

gradle_add_aar(kosmeditorcontroller_aar BUILDFILE ${CMAKE_CURRENT_SOURCE_DIR}/build.gradle NAME KOSMEditorController)
gradle_install_aar(kosmeditorcontroller_aar DESTINATION jar)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/KOSMEditorController-android-dependencies.xml
    ${CMAKE_BINARY_DIR}/lib/KOSMEditorController_${CMAKE_ANDROID_ARCH_ABI}-android-dependencies.xml
)
install(FILES KOSMEditorController-android-dependencies.xml DESTINATION ${KDE_INSTALL_LIBDIR} RENAME KOSMEditorController_${CMAKE_ANDROID_ARCH_ABI}-android-dependencies.xml)
