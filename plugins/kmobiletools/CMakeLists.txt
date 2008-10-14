
include_directories(${CMAKE_SOURCE_DIR}/kmobiletools/src)

########### next target ###############

set(kontact_kmobiletools_PART_SRCS kmobiletools_plugin.cpp)

qt4_add_dbus_interfaces(kontact_kmobiletools_PART_SRCS ${CMAKE_SOURCE_DIR}/kmobiletools/kmobiletools/libkmobiletools/org.kde.kmobiletools.main.xml)

kde4_add_plugin(kontact_kmobiletoolsplugin ${kontact_kmobiletools_PART_SRCS})

kdepim4_link_unique_libraries(kontact_kmobiletoolsplugin  ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} kmobiletoolslib kontactinterfaces)

########### install files ###############

install(TARGETS kontact_kmobiletoolsplugin DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES  kmobiletools.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kontact)

install(FILES kmobiletools.setdlg DESTINATION ${DATA_INSTALL_DIR}/kontact/ksettingsdialog)
