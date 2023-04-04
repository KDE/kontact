configure_file(kontact-version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/kontact-version.h @ONLY)
add_subdirectory(about)
add_library(kontactprivate)

########### next target ###############

target_sources(kontactprivate PRIVATE
    webengine/introductionwebenginepage.cpp
    webengine/introductionwebengineview.cpp
    mainwindow.cpp
    sidepanebase.cpp
    iconsidepane.cpp
    ksettingsdialog/kontactkcmultidialog.cpp
    ksettingsdialog/kontactsettingsdialog.cpp
    kontactconfiguredialog.cpp
    webengine/introductionwebenginepage.h
    webengine/introductionwebengineview.h
    mainwindow.h
    sidepanebase.h
    iconsidepane.h
    ksettingsdialog/kontactkcmultidialog.h
    ksettingsdialog/kontactsettingsdialog.h
    ksettingsdialog/kontactkcmultidialog_p.h
    ksettingsdialog/kontactsettingsdialog_p.h
    kontactconfiguredialog.h
    kontact.qrc
    )

ecm_qt_declare_logging_category(kontactprivate HEADER kontact_debug.h IDENTIFIER KONTACT_LOG CATEGORY_NAME org.kde.pim.kontact
        DESCRIPTION "kontact (kontact application)"
        OLD_CATEGORY_NAMES log_kontact
        EXPORT KONTACT
    )

kconfig_add_kcfg_files(kontactprivate prefs.kcfgc)


if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(kontactprivate PROPERTIES UNITY_BUILD ON)
endif()

generate_export_header(kontactprivate BASE_NAME kontact)

target_link_libraries(kontactprivate
    PUBLIC
    KF${KF_MAJOR_VERSION}::IconThemes
    PRIVATE
    KF${KF_MAJOR_VERSION}::KCMUtils
    KF${KF_MAJOR_VERSION}::I18n
    KF${KF_MAJOR_VERSION}::KIOWidgets
    KPim${KF_MAJOR_VERSION}::KontactInterface
    KF${KF_MAJOR_VERSION}::GuiAddons
    KPim${KF_MAJOR_VERSION}::Libkdepim
    KPim${KF_MAJOR_VERSION}::PimCommon
    KPim${KF_MAJOR_VERSION}::GrantleeTheme
    Qt::WebEngineWidgets
    )

set_target_properties(kontactprivate PROPERTIES
    VERSION ${KDEPIM_LIB_VERSION}
    SOVERSION ${KDEPIM_LIB_SOVERSION}
    )

install(TARGETS kontactprivate ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)

########### next target ###############


file(GLOB icons "${CMAKE_CURRENT_SOURCE_DIR}/icons/*-apps-kontact.png")
ecm_add_app_icon(kontact_bin_SRCS ICONS ${icons})

add_executable(kontact_bin)
target_sources(kontact_bin PRIVATE main.cpp kontact_debug.cpp)

set_target_properties(kontact_bin PROPERTIES OUTPUT_NAME kontact)

target_link_libraries(kontact_bin
    kontactprivate
    KF${KF_MAJOR_VERSION}::I18n
    KPim${KF_MAJOR_VERSION}::KontactInterface
    KF${KF_MAJOR_VERSION}::Crash
    )
if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(kontact_bin PROPERTIES UNITY_BUILD ON)
endif()

install(TARGETS kontact_bin ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

########### next target ###############

add_library(kcm_kontact MODULE kcmkontact.cpp kcmkontact.h)
target_link_libraries(kcm_kontact
    KF${KF_MAJOR_VERSION}::I18n
    KF${KF_MAJOR_VERSION}::KCMUtils
    KPim${KF_MAJOR_VERSION}::Libkdepim
    kontactprivate
    KF${KF_MAJOR_VERSION}::KIOWidgets
    KPim${KF_MAJOR_VERSION}::KontactInterface
    )

install(TARGETS kcm_kontact DESTINATION ${KDE_INSTALL_PLUGINDIR}/pim6/kcms/kontact)

########### install files ###############

install(PROGRAMS data/org.kde.kontact.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES data/org.kde.kontact.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install(FILES kontact.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
configure_file(data/org.kde.kontact.service.in org.kde.kontact.service)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.kontact.service DESTINATION ${KDE_INSTALL_DBUSSERVICEDIR})

add_subdirectory(icons)

