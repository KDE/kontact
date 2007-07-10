
include_directories( ${CMAKE_SOURCE_DIR}/kontact/interfaces  )


########### next target ###############

set(kontact_kaddressbookplugin_PART_SRCS kaddressbook_plugin.cpp )

qt4_add_dbus_interfaces(kontact_kaddressbookplugin_PART_SRCS ${CMAKE_SOURCE_DIR}/kaddressbook/org.kde.KAddressbook.Core.xml )

qt4_add_dbus_interfaces(kontact_kaddressbookplugin_PART_SRCS ${CMAKE_SOURCE_DIR}/kmail/org.kde.kmail.kmail.xml)


kde4_add_plugin(kontact_kaddressbookplugin WITH_PREFIX ${kontact_kaddressbookplugin_PART_SRCS})



target_link_libraries(kontact_kaddressbookplugin  ${KDE4_KDECORE_LIBS} ${KDE4_KABC_LIBS} kpinterfaces kdepim )

install(TARGETS kontact_kaddressbookplugin  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kaddressbookplugin.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kontact)

