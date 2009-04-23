
########### next target ###############

set(kontact_kaddressbookplugin_PART_SRCS kaddressbook_plugin.cpp)

qt4_add_dbus_interfaces(kontact_kaddressbookplugin_PART_SRCS ${CMAKE_SOURCE_DIR}/kaddressbook/org.kde.KAddressbook.Core.xml)
qt4_add_dbus_interfaces(kontact_kaddressbookplugin_PART_SRCS ${CMAKE_BINARY_DIR}/kmail/org.kde.kmail.kmail.xml)

kde4_add_plugin(kontact_kaddressbookplugin ${kontact_kaddressbookplugin_PART_SRCS})
add_dependencies(kontact_kaddressbookplugin kmail_xml)

kdepim4_link_unique_libraries(kontact_kaddressbookplugin  ${KDE4_KDECORE_LIBS} ${KDE4_KABC_LIBS} ${QT_QTGUI_LIBRARY} kontactinterfaces kdepim)



########### install files ###############

install(TARGETS kontact_kaddressbookplugin  DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES kaddressbookplugin.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kontact)

install(FILES kaddressbook.setdlg DESTINATION ${DATA_INSTALL_DIR}/kontact/ksettingsdialog)

