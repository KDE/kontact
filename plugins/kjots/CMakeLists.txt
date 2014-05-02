
include_directories(${CMAKE_SOURCE_DIR}/kjots ${CMAKE_BINARY_DIR}/kjots)

set(kontact_kjotsplugin_PART_SRCS
  kjots_plugin.cpp
)

qt5_add_dbus_interfaces(kontact_kjotsplugin_PART_SRCS ${CMAKE_BINARY_DIR}/kjots/org.kde.KJotsWidget.xml)

add_library(kontact_kjotsplugin MODULE ${kontact_kjotsplugin_PART_SRCS})

# We need the generated file org.kde.KJotsWidget.xml to exist
add_dependencies(kontact_kjotsplugin kjots)

target_link_libraries(kontact_kjotsplugin  KF5::KontactInterface)

install(TARGETS kontact_kjotsplugin DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES kjots_plugin.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kontact)

install(FILES kjots.setdlg DESTINATION ${DATA_INSTALL_DIR}/kontact/ksettingsdialog)

