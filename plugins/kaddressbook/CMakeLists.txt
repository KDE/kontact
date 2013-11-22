
include_directories(${CMAKE_SOURCE_DIR}/kaddressbook)

set(kontact_kaddressbookplugin_PART_SRCS
  kaddressbook_plugin.cpp
)

qt4_add_dbus_interfaces(kontact_kaddressbookplugin_PART_SRCS ${CMAKE_SOURCE_DIR}/kaddressbook/org.kde.kaddressbook.xml)


kde4_add_plugin(kontact_kaddressbookplugin ${kontact_kaddressbookplugin_PART_SRCS})

target_link_libraries(kontact_kaddressbookplugin ${KDE4_KPARTS_LIBS} ${KDEPIMLIBS_KONTACTINTERFACE_LIBS})

install(TARGETS kontact_kaddressbookplugin DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES kaddressbookplugin.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kontact)

install(FILES kaddressbook.setdlg DESTINATION ${DATA_INSTALL_DIR}/kontact/ksettingsdialog)

