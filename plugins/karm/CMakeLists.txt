
include_directories( ${CMAKE_SOURCE_DIR}/kontact/interfaces ${CMAKE_SOURCE_DIR}/ktimetracker  )


########### next target ###############

set(kontact_karm_PART_SRCS karm_plugin.cpp )


qt4_add_dbus_interfaces(kontact_karm_PART_SRCS ${CMAKE_SOURCE_DIR}/ktimetracker/org.kde.karm.Karm.xml )

kde4_add_plugin(kontact_karm WITH_PREFIX ${kontact_karm_PART_SRCS})



target_link_libraries(kontact_karm  ${KDE4_KDECORE_LIBS} kpinterfaces )

install(TARGETS kontact_karm  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES karmplugin.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kontact)

