
########### next target ###############

set(kontact_weatherplugin_PART_SRCS weather_plugin.cpp summarywidget.cpp)

qt4_add_dbus_interfaces(kontact_weatherplugin_PART_SRCS org.kde.kweather.service.xml)

kde4_add_plugin(kontact_weatherplugin ${kontact_weatherplugin_PART_SRCS})

target_link_libraries(kontact_weatherplugin ${KDE4_KPARTS_LIBS} kontactinterfaces kdepim)

########### install files ###############

install(TARGETS kontact_weatherplugin  DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES weatherplugin.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kontact)

install(FILES weather.setdlg DESTINATION ${DATA_INSTALL_DIR}/kontact/ksettingsdialog)

