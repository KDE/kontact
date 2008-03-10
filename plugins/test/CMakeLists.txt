
include_directories( ${CMAKE_SOURCE_DIR}/kontact/interfaces  )


########### next target ###############

set(kptestplugin_PART_SRCS test_plugin.cpp test_part.cpp )


#kde4_add_dcop_stubs(kptestplugin_PART_SRCS kaddressbookiface.h )

kde4_add_plugin(kontact_testplugin ${kptestplugin_PART_SRCS})



kdepim4_link_unique_libraries(kontact_testplugin ${KDE4_KPARTS_LIBS} kpinterfaces)

install(TARGETS kontact_testplugin  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kptestplugin.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES kptestplugin.rc  DESTINATION ${DATA_INSTALL_DIR}/kptestplugin)
install( FILES testpartui.rc  DESTINATION ${DATA_INSTALL_DIR}/testpart)
