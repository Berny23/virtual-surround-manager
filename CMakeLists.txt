cmake_minimum_required(VERSION 3.28)

project("Virtual Surround Manager" VERSION 1.0 LANGUAGES CXX)

set(APPLICATION_ID "de.berny23.virtual_surround_manager")
set(APPLICATION_NAME "Virtual Surround Sound")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(IS_FLATPAK "Some things need to be handled differently in C++ if running in a Flatpak" OFF)

# Kirigami dependencies
find_package(ECM 6.0.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

set(CMAKE_AUTOMOC ON)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMFindQmlModule)
include(ECMQmlModule)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Quick
    Gui
    QuickControls2
    Widgets
)
find_package(KF6 REQUIRED COMPONENTS
    Kirigami
    I18n
    CoreAddons
    QQC2DesktopStyle
    IconThemes
    Config
)

ecm_find_qmlmodule(org.kde.kirigami REQUIRED)

find_package(PkgConfig REQUIRED)

# PipeWire dependencies
pkg_check_modules(PIPEWIRE REQUIRED libpipewire-0.3)

# Translations
ki18n_install(po)

# Include CMakeLists.txt in src directory
add_subdirectory(src)

# Prepare and install shortcut to default system application path (e.g. /usr/share/applications/)
configure_file(dist/${APPLICATION_ID}.desktop.in
    ${CMAKE_CURRENT_BINARY_DIR}/${APPLICATION_ID}.desktop @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${APPLICATION_ID}.desktop
    DESTINATION ${KDE_INSTALL_APPDIR})

# Prepare and install MetaInfo file for app stores
configure_file(dist/${APPLICATION_ID}.metainfo.xml.in
    ${CMAKE_CURRENT_BINARY_DIR}/${APPLICATION_ID}.metainfo.xml)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${APPLICATION_ID}.metainfo.xml
    DESTINATION ${KDE_INSTALL_METAINFODIR}
)

# From kirigami docs
feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
