
include_directories( ${CMAKE_SOURCE_DIR}/kontact/interfaces  )


########### next target ###############

set(kptestplugin_PART_SRCS test_plugin.cpp test_part.cpp )


#kde4_add_dcop_stubs(kptestplugin_PART_SRCS kaddressbookiface.h )

kde4_add_plugin(kptestplugin ${kptestplugin_PART_SRCS})



kdepim4_link_unique_libraries(kptestplugin ${KDE4_KPARTS_LIBS} kpinterfaces)

install(TARGETS kptestplugin  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kptestplugin.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES kptestplugin.rc  DESTINATION ${DATA_INSTALL_DIR}/kptestplugin)
install( FILES testpartui.rc  DESTINATION ${DATA_INSTALL_DIR}/testpart)
