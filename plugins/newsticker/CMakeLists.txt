
include_directories( ${CMAKE_SOURCE_DIR}/kontact/interfaces ${CMAKE_SOURCE_DIR}/libkdepim  )


########### next target ###############

set(kontact_newstickerplugin_PART_SRCS newsticker_plugin.cpp summarywidget.cpp )


kde4_add_plugin(kontact_newstickerplugin WITH_PREFIX ${kontact_newstickerplugin_PART_SRCS})



target_link_libraries(kontact_newstickerplugin  ${KDE4_KPARTS_LIBS} kdepim kpinterfaces )

install(TARGETS kontact_newstickerplugin  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kcm_kontactknt_PART_SRCS kcmkontactknt.cpp )


kde4_add_plugin(kcm_kontactknt ${kcm_kontactknt_PART_SRCS})



target_link_libraries(kcm_kontactknt  ${KDE4_KDEUI_LIBS} ${KDE4_KDE3SUPPORT_LIBS})

install(TARGETS kcm_kontactknt  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES newstickerplugin.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kontact)
install( FILES kcmkontactknt.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

