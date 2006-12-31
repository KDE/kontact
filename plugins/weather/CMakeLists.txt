
include_directories( ${CMAKE_SOURCE_DIR}/kontact/interfaces ${CMAKE_SOURCE_DIR}/libkdepim  )


########### next target ###############

set(kontact_weatherplugin_PART_SRCS weather_plugin.cpp summarywidget.cpp )

qt4_add_dbus_interfaces( kontact_weatherplugin_PART_SRCS ${DBUS_INTERFACES_DIR}/org.kde.kweather.service.xml )

kde4_automoc(${kontact_weatherplugin_PART_SRCS})

kde4_add_plugin(kontact_weatherplugin WITH_PREFIX ${kontact_weatherplugin_PART_SRCS})



target_link_libraries(kontact_weatherplugin ${KDE4_KDECORE_LIBS} ${KDE4_KPARTS_LIBS} kpinterfaces kdepim)

install(TARGETS kontact_weatherplugin  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES weatherplugin.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kontact)

