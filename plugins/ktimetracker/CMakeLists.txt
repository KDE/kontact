
include_directories(${CMAKE_SOURCE_DIR}/ktimetracker)

########### next target ###############

set(kontact_karm_PART_SRCS ktimetracker_plugin.cpp)

qt4_add_dbus_interfaces(kontact_karm_PART_SRCS ${CMAKE_SOURCE_DIR}/ktimetracker/org.kde.ktimetracker.ktimetracker.xml)

kde4_add_plugin(kontact_karmplugin ${kontact_karm_PART_SRCS})

kdepim4_link_unique_libraries(kontact_karmplugin  ${KDE4_KDECORE_LIBS} kontactinterfaces)

########### install files ###############

install(TARGETS kontact_karmplugin  DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES ktimetracker_plugin.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kontact)

install(FILES ktimetracker.setdlg DESTINATION ${DATA_INSTALL_DIR}/kontact/ksettingsdialog)
