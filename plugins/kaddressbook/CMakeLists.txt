
set(kontact_kaddressbookplugin_PART_SRCS
  kaddressbook_plugin.cpp
)

kde4_add_plugin(kontact_kaddressbookplugin ${kontact_kaddressbookplugin_PART_SRCS} )

target_link_libraries(kontact_kaddressbookplugin ${KDE4_KPARTS_LIBS} kontactinterfaces)

install(TARGETS kontact_kaddressbookplugin  DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES kaddressbook_plugin.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kontact)

install(FILES kaddressbook.setdlg DESTINATION ${DATA_INSTALL_DIR}/kontact/ksettingsdialog)

