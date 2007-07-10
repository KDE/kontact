
include_directories( ${CMAKE_SOURCE_DIR}/kontact/interfaces ${CMAKE_SOURCE_DIR}/akregator/src  )


########### next target ###############

set(kontact_akregator_PART_SRCS akregator_plugin.cpp )

qt4_add_dbus_interfaces(kontact_akregator_PART_SRCS ${CMAKE_SOURCE_DIR}/akregator/src/org.kde.akregator.part.xml ) 


kde4_add_plugin(kontact_akregator WITH_PREFIX ${kontact_akregator_PART_SRCS})



target_link_libraries(kontact_akregator ${KDE4_KPARTS_LIBS} kpinterfaces)

install(TARGETS kontact_akregator  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES akregatorplugin.desktop akregatorplugin3.2.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kontact)

