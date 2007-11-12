
include_directories( ${CMAKE_SOURCE_DIR}/kontact/interfaces ${CMAKE_SOURCE_DIR}/ktimetracker  )


########### next target ###############

set(kontact_karm_PART_SRCS karm_plugin.cpp )


qt4_add_dbus_interfaces(kontact_karm_PART_SRCS ${CMAKE_SOURCE_DIR}/ktimetracker/org.kde.ktimetracker.ktimetracker.xml )

kde4_add_plugin(kontact_karm ${kontact_karm_PART_SRCS})



kdepim4_link_unique_libraries(kontact_karm  ${KDE4_KDECORE_LIBS} kpinterfaces )

install(TARGETS kontact_karm  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES karmplugin.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kontact)

