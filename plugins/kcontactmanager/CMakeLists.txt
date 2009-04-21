
set(kontact_kcontactmanagerplugin_PART_SRCS
  kcontactmanager_plugin.cpp
)

kde4_add_plugin(kontact_kcontactmanagerplugin ${kontact_kcontactmanagerplugin_PART_SRCS} )

target_link_libraries(kontact_kcontactmanagerplugin ${KDE4_KPARTS_LIBS} kontactinterfaces)

install(TARGETS kontact_kcontactmanagerplugin  DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES kcontactmanager_plugin.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kontact)

install(FILES kcontactmanager.setdlg DESTINATION ${DATA_INSTALL_DIR}/kontact/ksettingsdialog)

