
include_directories(
  ${CMAKE_SOURCE_DIR}/akregator/src
  ${Boost_INCLUDE_DIRS}
)

########### next target ###############

set(kontact_akregator_PART_SRCS akregator_plugin.cpp)

qt4_add_dbus_interfaces(kontact_akregator_PART_SRCS ${CMAKE_SOURCE_DIR}/akregator/src/org.kde.akregator.part.xml)

add_library(kontact_akregatorplugin MODULE ${kontact_akregator_PART_SRCS})

target_link_libraries(kontact_akregatorplugin ${KDE4_KPARTS_LIBS} KF5::KontactInterface)

########### install files ###############

install(TARGETS kontact_akregatorplugin DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES akregatorplugin.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kontact)

install(FILES akregator.setdlg DESTINATION ${DATA_INSTALL_DIR}/kontact/ksettingsdialog)

