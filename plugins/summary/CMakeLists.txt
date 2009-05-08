
########### next target ###############

set(kontact_summaryplugin_PART_SRCS summaryview_plugin.cpp summaryview_part.cpp dropwidget.cpp)
qt4_add_resources(kontact_summaryplugin_PART_SRCS summaryview.qrc )

if(KMAIL_SUPPORTED)
   qt4_add_dbus_interfaces(kontact_summaryplugin_PART_SRCS ${CMAKE_BINARY_DIR}/kmail/org.kde.kmail.kmail.xml)
endif(KMAIL_SUPPORTED)

kde4_add_plugin(kontact_summaryplugin ${kontact_summaryplugin_PART_SRCS})
if(KMAIL_SUPPORTED)
   add_dependencies(kontact_summaryplugin kmail_xml)
endif(KMAIL_SUPPORTED)

target_link_libraries(kontact_summaryplugin ${KDE4_KUTILS_LIBS} ${KDE4_KPARTS_LIBS} ${KDEPIMLIBS_KPIMIDENTITIES_LIBS} kontactinterfaces kdepim)

########### next target ###############

set(kcm_kontactsummary_PART_SRCS kcmkontactsummary.cpp)

kde4_add_plugin(kcm_kontactsummary ${kcm_kontactsummary_PART_SRCS})

target_link_libraries(kcm_kontactsummary  ${KDE4_KDEUI_LIBS})

########### install files ###############

install(TARGETS kontact_summaryplugin  DESTINATION ${PLUGIN_INSTALL_DIR})
install(TARGETS kcm_kontactsummary  DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES summaryplugin.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kontact)
install(FILES kcmkontactsummary.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES kontactsummary_part.rc  DESTINATION ${DATA_INSTALL_DIR}/kontactsummary)

install(FILES summary.setdlg DESTINATION ${DATA_INSTALL_DIR}/kontact/ksettingsdialog)
