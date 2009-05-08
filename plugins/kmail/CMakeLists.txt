
include_directories(${CMAKE_SOURCE_DIR}/kmail ${CMAKE_BINARY_DIR}/kmail)

########### next target ###############

set(interface_SRCS)

set(folder_adaptor ${CMAKE_SOURCE_DIR}/kmail/folderadaptor.h)
set(folder_adaptor_xml ${CMAKE_CURRENT_BINARY_DIR}/org.kde.kmail.folder.xml)

qt4_generate_dbus_interface(${folder_adaptor} org.kde.kmail.folder.xml)
qt4_add_dbus_interface(interface_SRCS ${folder_adaptor_xml} kmail_folder_interface)

set(kontact_kmail_plugins_interface_SRCS ${interface_SRCS})
qt4_add_dbus_interfaces(kontact_kmail_plugins_interface_SRCS ${CMAKE_BINARY_DIR}/kmail/org.kde.kmail.kmail.xml)

set(kontact_kmailplugin_PART_SRCS kmail_plugin.cpp summarywidget.cpp ${kontact_kmail_plugins_interface_SRCS})

kde4_add_plugin(kontact_kmailplugin ${kontact_kmailplugin_PART_SRCS})
add_dependencies(kontact_kmailplugin kmail)

target_link_libraries(kontact_kmailplugin ${KDE4_KPARTS_LIBS} kontactinterfaces kdepim ${KDEPIMLIBS_KCAL_LIBS})

########### next target ###############

set(kcm_kmailsummary_PART_SRCS kcmkmailsummary.cpp ${kontact_kmail_plugins_interface_SRCS})

kde4_add_plugin(kcm_kmailsummary ${kcm_kmailsummary_PART_SRCS})

target_link_libraries(kcm_kmailsummary ${KDE4_KDEUI_LIBS})

########### install files ###############

install(TARGETS kontact_kmailplugin  DESTINATION ${PLUGIN_INSTALL_DIR})

install(TARGETS kcm_kmailsummary  DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES kmailplugin.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kontact)

install( FILES kcmkmailsummary.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES kmail.setdlg DESTINATION ${DATA_INSTALL_DIR}/kontact/ksettingsdialog)
