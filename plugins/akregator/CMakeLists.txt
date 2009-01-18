
include_directories(${CMAKE_SOURCE_DIR}/akregator/src)
include_directories( ${Boost_INCLUDE_DIRS} )

########### next target ###############

set(kontact_akregator_PART_SRCS akregator_plugin.cpp)

qt4_add_dbus_interfaces(kontact_akregator_PART_SRCS ${CMAKE_SOURCE_DIR}/akregator/src/org.kde.akregator.part.xml) 


kde4_add_plugin(kontact_akregatorplugin ${kontact_akregator_PART_SRCS})


target_link_libraries(kontact_akregatorplugin ${KDE4_KPARTS_LIBS} kontactinterfaces)

########### install files ###############

install(TARGETS kontact_akregatorplugin  DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES akregatorplugin.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kontact)

install(FILES akregator.setdlg DESTINATION ${DATA_INSTALL_DIR}/kontact/ksettingsdialog)

