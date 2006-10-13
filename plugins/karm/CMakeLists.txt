
include_directories( ${CMAKE_SOURCE_DIR}/kontact/interfaces ${CMAKE_SOURCE_DIR}/karm  )


########### next target ###############

set(kontact_karm_PART_SRCS karm_plugin.cpp )

kde4_automoc(kontact_karm ${kontact_karm_PART_SRCS})

qt4_add_dbus_interfaces(kontact_karm_PART_SRCS ${CMAKE_SOURCE_DIR}/karm/org.kde.karm.Karm.xml )

kde4_add_plugin(kontact_karm WITH_PREFIX ${kontact_karm_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kontact_karm )

target_link_libraries(kontact_karm  ${KDE4_KDECORE_LIBS} kpinterfaces )

install(TARGETS kontact_karm  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES karmplugin.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kontact)

