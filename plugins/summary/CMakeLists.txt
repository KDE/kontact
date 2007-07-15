
include_directories(
    ${CMAKE_SOURCE_DIR}/kontact/interfaces
    ${CMAKE_SOURCE_DIR}/libkdepim
)


########### next target ###############

set(kontact_summaryplugin_PART_SRCS summaryview_plugin.cpp summaryview_part.cpp dropwidget.cpp )


kde4_add_plugin(kontact_summaryplugin WITH_PREFIX ${kontact_summaryplugin_PART_SRCS})



target_link_libraries(kontact_summaryplugin ${KDE4_KUTILS_LIBS} ${KDE4_KPARTS_LIBS} ${KDE4_KPIMIDENTITIES_LIBS} kpinterfaces kdepim)

install(TARGETS kontact_summaryplugin  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kcm_kontactsummary_PART_SRCS kcmkontactsummary.cpp )


kde4_add_plugin(kcm_kontactsummary ${kcm_kontactsummary_PART_SRCS})



target_link_libraries(kcm_kontactsummary  ${KDE4_KUTILS_LIBS} ${KDE4_KDE3SUPPORT_LIBS})

install(TARGETS kcm_kontactsummary  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES summaryplugin.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kontact)
install( FILES kcmkontactsummary.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES kontactsummary_part.rc  DESTINATION ${DATA_INSTALL_DIR}/kontactsummary)

