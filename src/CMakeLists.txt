configure_file(kontact-version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/kontact-version.h @ONLY)
add_subdirectory(about)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    )

########### next target ###############

set(kontactprivate_LIB_SRCS
    webengine/introductionwebenginepage.cpp
    webengine/introductionwebengineview.cpp
    mainwindow.cpp
    sidepanebase.cpp
    iconsidepane.cpp
    aboutdialog.cpp
    kontactconfiguredialog.cpp
    )

ecm_qt_declare_logging_category(kontactprivate_LIB_SRCS HEADER kontact_debug.h IDENTIFIER KONTACT_LOG CATEGORY_NAME org.kde.pim.kontact
        DESCRIPTION "kontact (kontact application)"
        OLD_CATEGORY_NAMES log_kontact
        EXPORT KONTACT
    )

kconfig_add_kcfg_files(kontactprivate_LIB_SRCS prefs.kcfgc)
qt5_add_resources(kontactprivate_LIB_SRCS kontact.qrc)

add_library(kontactprivate ${kontactprivate_LIB_SRCS})
if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(kontactprivate PROPERTIES UNITY_BUILD ON)
endif()

generate_export_header(kontactprivate BASE_NAME kontact)

target_link_libraries(kontactprivate
    PUBLIC
    KF5::WindowSystem
    KF5::IconThemes
    PRIVATE
    KF5::KIOWidgets
    KF5::KontactInterface
    KF5::KCMUtils
    KF5::Libkdepim
    KF5::PimCommon
    KF5::GrantleeTheme
    Qt5::WebEngine
    Qt5::WebEngineWidgets
    )

set_target_properties(kontactprivate PROPERTIES
    VERSION ${KDEPIM_LIB_VERSION}
    SOVERSION ${KDEPIM_LIB_SOVERSION}
    )

install(TARGETS kontactprivate ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)

########### next target ###############

set(kontact_bin_SRCS main.cpp kontact_debug.cpp)

file(GLOB icons "${CMAKE_CURRENT_SOURCE_DIR}/icons/*-apps-kontact.png")
ecm_add_app_icon(kontact_bin_SRCS ICONS ${icons})

add_executable(kontact_bin ${kontact_bin_SRCS})

set_target_properties(kontact_bin PROPERTIES OUTPUT_NAME kontact)

target_link_libraries(kontact_bin
    KF5::Service
    kontactprivate
    KF5::KontactInterface
    KF5::Crash
    )
if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(kontact_bin PROPERTIES UNITY_BUILD ON)
endif()

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

install(PROGRAMS data/org.kde.kontact.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES data/org.kde.kontact.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install(FILES data/kontactconfig.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install(FILES kontact.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
configure_file(data/org.kde.kontact.service.in org.kde.kontact.service)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.kontact.service DESTINATION ${KDE_INSTALL_DBUSSERVICEDIR})

add_subdirectory(icons)

