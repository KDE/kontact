configure_file(kontact-version.h.in ${CMAKE_CURRENT_BINARY_DIR}/kontact-version.h @ONLY)
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
    KF6::IconThemes
    PRIVATE
    KF6::KCMUtils
    KF6::I18n
    KF6::KIOWidgets
    KPim6::KontactInterface
    KF6::GuiAddons
    KPim6::Libkdepim
    KPim6::PimCommon
    KPim6::GrantleeTheme
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
    KF6::I18n
    KPim6::KontactInterface
    KF6::Crash
    )
if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(kontact_bin PROPERTIES UNITY_BUILD ON)
endif()

install(TARGETS kontact_bin ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

########### next target ###############

add_library(kcm_kontact MODULE kcmkontact.cpp kcmkontact.h)
target_link_libraries(kcm_kontact
    KF6::I18n
    KF6::KCMUtils
    KPim6::Libkdepim
    kontactprivate
    KF6::KIOWidgets
    KPim6::KontactInterface
    )

install(TARGETS kcm_kontact DESTINATION ${KDE_INSTALL_PLUGINDIR}/pim6/kcms/kontact)

########### install files ###############

install(PROGRAMS data/org.kde.kontact.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES data/org.kde.kontact.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install(FILES kontact.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
configure_file(data/org.kde.kontact.service.in org.kde.kontact.service)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.kontact.service DESTINATION ${KDE_INSTALL_DBUSSERVICEDIR})

add_subdirectory(icons)

