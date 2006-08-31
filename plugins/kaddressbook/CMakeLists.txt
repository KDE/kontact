
include_directories( ${CMAKE_SOURCE_DIR}/kontact/interfaces  )


########### next target ###############

set(kontact_kaddressbookplugin_PART_SRCS kaddressbook_plugin.cpp )

kde4_automoc(${kontact_kaddressbookplugin_PART_SRCS})

#kde4_add_dcop_stubs(kontact_kaddressbookplugin_PART_SRCS 
#	${CMAKE_SOURCE_DIR}/kaddressbook/kaddressbookiface.h 
#	${CMAKE_SOURCE_DIR}/kmail/kmailIface.h )

kde4_add_plugin(kontact_kaddressbookplugin WITH_PREFIX ${kontact_kaddressbookplugin_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kontact_kaddressbookplugin )

target_link_libraries(kontact_kaddressbookplugin  ${KDE4_KDECORE_LIBS} ${KDE4_KABC_LIBS} kpinterfaces kdepim )

install(TARGETS kontact_kaddressbookplugin  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kaddressbookplugin.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kontact)

