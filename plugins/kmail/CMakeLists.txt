include_directories(
  ${CMAKE_SOURCE_DIR}/kmail
  ${CMAKE_BINARY_DIR}/kmail
  ${CMAKE_SOURCE_DIR}/messageviewer
  ${CMAKE_SOURCE_DIR}/mailcommon
  ${Boost_INCLUDE_DIRS}
)

########### next target ###############
set(interface_SRCS)

set(kontact_kmail_plugins_interface_SRCS ${interface_SRCS})
qt4_add_dbus_interfaces(kontact_kmail_plugins_interface_SRCS ${CMAKE_BINARY_DIR}/kmail/org.kde.kmail.kmail.xml)

set(kontact_kmailplugin_PART_SRCS kmail_plugin.cpp summarywidget.cpp ${kontact_kmail_plugins_interface_SRCS})

kde4_add_plugin(kontact_kmailplugin ${kontact_kmailplugin_PART_SRCS})
add_dependencies(kontact_kmailplugin kmail_xml)

target_link_libraries(kontact_kmailplugin ${KDEPIMLIBS_KMIME_LIBS} ${KDE4_KPARTS_LIBS} ${KDEPIMLIBS_KONTACTINTERFACE_LIBS} ${KDEPIMLIBS_KCALCORE_LIBS} ${KDEPIMLIBS_KCALUTILS_LIBS} kdepim)

########### next target ###############

set(kcm_kmailsummary_PART_SRCS kcmkmailsummary.cpp ${kontact_kmail_plugins_interface_SRCS})

kde4_add_plugin(kcm_kmailsummary ${kcm_kmailsummary_PART_SRCS})
add_dependencies(kcm_kmailsummary kmail_xml)

target_link_libraries(kcm_kmailsummary ${KDE4_KDEUI_LIBS} ${KDEPIMLIBS_KMIME_LIBS} ${KDEPIMLIBS_AKONADI_LIBS} akonadi_next mailcommon)

########### install files ###############

install(TARGETS kontact_kmailplugin DESTINATION ${PLUGIN_INSTALL_DIR})

install(TARGETS kcm_kmailsummary DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES kmailplugin.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kontact)

install(FILES kcmkmailsummary.desktop DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES kmail.setdlg DESTINATION ${DATA_INSTALL_DIR}/kontact/ksettingsdialog)
