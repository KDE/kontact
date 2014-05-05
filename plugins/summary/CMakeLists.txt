
include_directories(${Boost_INCLUDE_DIRS})

########### next target ###############

set(kontact_summaryplugin_PART_SRCS summaryview_plugin.cpp summaryview_part.cpp dropwidget.cpp)
qt4_add_resources(kontact_summaryplugin_PART_SRCS summaryview.qrc)

if(KMAIL_SUPPORTED)
  qt5_add_dbus_interfaces(kontact_summaryplugin_PART_SRCS ${CMAKE_BINARY_DIR}/kmail/org.kde.kmail.kmail.xml)
endif()

add_library(kontact_summaryplugin MODULE ${kontact_summaryplugin_PART_SRCS})
if(KMAIL_SUPPORTED)
  add_dependencies(kontact_summaryplugin kmail_xml)
endif()

target_link_libraries(kontact_summaryplugin KF5::KCMUtils  KF5::PimIdentities KF5::KontactInterface kdepim)

########### next target ###############

set(kcm_kontactsummary_PART_SRCS kcmkontactsummary.cpp)

add_library(kcm_kontactsummary MODULE ${kcm_kontactsummary_PART_SRCS})

target_link_libraries(kcm_kontactsummary KF5::KCMUtils Qt5::Widgets KF5::KCMUtils KF5::KDELibs4Support KF5::KontactInterface)

########### install files ###############

install(TARGETS kontact_summaryplugin DESTINATION ${PLUGIN_INSTALL_DIR})
install(TARGETS kcm_kontactsummary DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES summaryplugin.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kontact)
install(FILES kcmkontactsummary.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES kontactsummary_part.rc DESTINATION ${DATA_INSTALL_DIR}/kontactsummary)

install(FILES summary.setdlg DESTINATION ${DATA_INSTALL_DIR}/kontact/ksettingsdialog)
