configure_file(config-enterprise.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-enterprise.h )
configure_file(config-kdepim.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kdepim.h )

configure_file(kontact-version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/kontact-version.h @ONLY)
add_subdirectory(about)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    )

########### next target ###############

set(kontactprivate_introduction_LIB_SRCS
    webengine/introductionwebenginepage.cpp
    webengine/introductionwebengineview.cpp
    )
set(kontactprivate_web_LIB Qt5::WebEngine Qt5::WebEngineWidgets)

set(kontactprivate_LIB_SRCS
    ${kontactprivate_introduction_LIB_SRCS}
    mainwindow.cpp
    sidepanebase.cpp
    iconsidepane.cpp
    aboutdialog.cpp
    kontactconfiguredialog.cpp
    )
ecm_qt_declare_logging_category(kontactprivate_LIB_SRCS HEADER kontact_debug.h IDENTIFIER KONTACT_LOG CATEGORY_NAME log_kontact)

kconfig_add_kcfg_files(kontactprivate_LIB_SRCS prefs.kcfgc)

add_library(kontactprivate ${kontactprivate_LIB_SRCS})
generate_export_header(kontactprivate BASE_NAME kontact)

target_link_libraries(kontactprivate
    PUBLIC
    KF5::WindowSystem
    KF5::IconThemes
    PRIVATE
    KF5::KIOWidgets
    KF5::KontactInterface
    KF5::KCMUtils
    ${kontactprivate_web_LIB}
    KF5::Libkdepim
    KF5::GrantleeTheme
    )

set_target_properties(kontactprivate PROPERTIES
    VERSION ${KDEPIM_LIB_VERSION}
    SOVERSION ${KDEPIM_LIB_SOVERSION}
    )

install(TARGETS kontactprivate ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)

########### next target ###############

set(kontact_bin_SRCS main.cpp kontact_debug.cpp)

file(GLOB ICONS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/icons/*-apps-kontact.png")
ecm_add_app_icon(kontact_bin_SRCS ICONS ${ICONS_SRCS})

add_executable(kontact_bin ${kontact_bin_SRCS})

set_target_properties(kontact_bin PROPERTIES OUTPUT_NAME kontact)

target_link_libraries(kontact_bin
    KF5::Service
    KF5::KdepimDBusInterfaces
    kontactprivate
    KF5::KontactInterface
    )

install(TARGETS kontact_bin ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

########### next target ###############

set(kcm_kontact_PART_SRCS kcmkontact.cpp)

add_library(kcm_kontact MODULE ${kcm_kontact_PART_SRCS})

target_link_libraries(kcm_kontact
    KF5::KCMUtils
    KF5::Libkdepim
    kontactprivate
    KF5::KIOWidgets
    KF5::KontactInterface
    )

install(TARGETS kcm_kontact DESTINATION ${KDE_INSTALL_PLUGINDIR})

########### install files ###############

install(PROGRAMS org.kde.kontact.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES org.kde.kontact.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install(FILES kontact.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
install(FILES kontactconfig.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install(FILES kontactui.rc DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/kontact)

add_subdirectory(icons)

