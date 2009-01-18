
include_directories( ${CMAKE_SOURCE_DIR}/knode  )

########### next target ###############

set(kontact_knodeplugin_PART_SRCS knode_plugin.cpp )

qt4_add_dbus_interfaces(kontact_knodeplugin_PART_SRCS ${CMAKE_SOURCE_DIR}/knode/org.kde.knode.xml)

kde4_add_plugin(kontact_knodeplugin ${kontact_knodeplugin_PART_SRCS})

target_link_libraries(kontact_knodeplugin ${KDE4_KPARTS_LIBS} kontactinterfaces)

########### install files ###############

install(TARGETS kontact_knodeplugin  DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES knodeplugin.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kontact)

install(FILES knode.setdlg DESTINATION ${DATA_INSTALL_DIR}/kontact/ksettingsdialog)
