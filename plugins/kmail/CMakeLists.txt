
include_directories(
	${CMAKE_SOURCE_DIR}/kontact/interfaces
	${CMAKE_SOURCE_DIR}/kmail
	${CMAKE_BINARY_DIR}/kmail
	${CMAKE_SOURCE_DIR}/libkdepim
	
)

########### next target ###############

set(kontact_kmail_plugins_interface_SRCS)
qt4_add_dbus_interfaces(kontact_kmail_plugins_interface_SRCS ${CMAKE_SOURCE_DIR}/kmail/org.kde.kmail.kmail.xml )

set(kontact_kmailplugin_PART_SRCS kmail_plugin.cpp summarywidget.cpp ${kontact_kmail_plugins_interface_SRCS})

kde4_automoc(${kontact_kmailplugin_PART_SRCS})


kde4_add_plugin(kontact_kmailplugin WITH_PREFIX ${kontact_kmailplugin_PART_SRCS})



target_link_libraries(kontact_kmailplugin ${KDE4_KDECORE_LIBS} ${KDE4_KPARTS_LIBS} kpinterfaces ${KDE4_KCAL_LIBS})

install(TARGETS kontact_kmailplugin  DESTINATION ${PLUGIN_INSTALL_DIR})
add_dependencies(kontact_kmailplugin kmailprivate)

########### next target ###############

set(kcm_kmailsummary_PART_SRCS kcmkmailsummary.cpp ${kontact_kmail_plugins_interface_SRCS} )

kde4_automoc(${kcm_kmailsummary_PART_SRCS})

kde4_add_plugin(kcm_kmailsummary ${kcm_kmailsummary_PART_SRCS})


target_link_libraries(kcm_kmailsummary  ${KDE4_KDEUI_LIBS} ${KDE4_KDE3SUPPORT_LIBS})

install(TARGETS kcm_kmailsummary  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kmailplugin.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kontact)
install( FILES kcmkmailsummary.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

