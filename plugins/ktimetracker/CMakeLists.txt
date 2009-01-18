
include_directories(${CMAKE_SOURCE_DIR}/ktimetracker)

########### next target ###############

set(kontact_ktimetracker_PART_SRCS ktimetracker_plugin.cpp)

qt4_add_dbus_interfaces(kontact_ktimetracker_PART_SRCS ${CMAKE_SOURCE_DIR}/ktimetracker/org.kde.ktimetracker.ktimetracker.xml)

kde4_add_plugin(kontact_ktimetrackerplugin ${kontact_ktimetracker_PART_SRCS})

target_link_libraries(kontact_ktimetrackerplugin  ${KDE4_KDEUI_LIBS} kontactinterfaces)

########### install files ###############

install(TARGETS kontact_ktimetrackerplugin  DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES ktimetracker_plugin.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kontact)

install(FILES ktimetracker.setdlg DESTINATION ${DATA_INSTALL_DIR}/kontact/ksettingsdialog)
