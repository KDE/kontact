
include_directories( ${CMAKE_SOURCE_DIR}/kontact/interfaces ${CMAKE_SOURCE_DIR}/akregator/src  )


########### next target ###############

set(kontact_akregator_PART_SRCS akregator_plugin.cpp )

kde4_automoc(kontact_akregator ${kontact_akregator_PART_SRCS})

#kde4_add_dcop_stubs(kontact_akregator_PART_SRCS ${CMAKE_SOURCE_DIR}/akregator/src/akregator_partiface.h )

kde4_add_plugin(kontact_akregator WITH_PREFIX ${kontact_akregator_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kontact_akregator )

target_link_libraries(kontact_akregator ${KDE4_KPARTS_LIBS} kpinterfaces)

install(TARGETS kontact_akregator  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES akregatorplugin.desktop akregatorplugin3.2.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kontact)

