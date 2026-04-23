add_executable(virtual-surround-manager)

# Kirigami libraries, settings and links
ecm_add_qml_module(virtual-surround-manager
    URI de.berny23.virtual-surround-manager
)

target_sources(virtual-surround-manager
    PRIVATE
    frontend_manager.h
    frontend_manager.cpp
    main.cpp
    pipewire_manager.h
    pipewire_manager.cpp
)

target_link_libraries(virtual-surround-manager
    PRIVATE
    Qt6::Quick
    Qt6::Qml
    Qt6::Gui
    Qt6::Widgets
    Qt6::QuickControls2
    KF6::I18n
    KF6::I18nQml
    KF6::CoreAddons
    KF6::IconThemes
    KF6::ConfigCore
)

# PipeWire libraries
target_include_directories(virtual-surround-manager PRIVATE ${PIPEWIRE_INCLUDE_DIRS})
target_compile_options(virtual-surround-manager PRIVATE ${PIPEWIRE_CFLAGS_OTHER})
target_link_libraries(virtual-surround-manager PRIVATE ${PIPEWIRE_LIBRARIES})

install(
    DIRECTORY assets/hrir_wav/
    DESTINATION ${KDE_INSTALL_DATADIR}/virtual-surround-manager/hrir_wav
    FILES_MATCHING
    PATTERN "*.wav"
    PATTERN "*.txt" # The license text for HeSuVi
)

install(
    FILES assets/icons/${APPLICATION_ID}.svg
    DESTINATION ${KDE_INSTALL_DATADIR}/icons/hicolor/scalable/apps
)

add_subdirectory(qml)

# Install binary to default system path
install(TARGETS virtual-surround-manager ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
