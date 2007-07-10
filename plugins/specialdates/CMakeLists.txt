
include_directories( ${CMAKE_SOURCE_DIR}/kontact/interfaces ${CMAKE_SOURCE_DIR}/libkdepim  )


########### next target ###############

set(kontact_specialdatesplugin_PART_SRCS specialdates_plugin.cpp sdsummarywidget.cpp )

qt4_add_dbus_interfaces(kontact_specialdatesplugin_PART_SRCS ${CMAKE_SOURCE_DIR}/kaddressbook/org.kde.KAddressbook.Core.xml )



kde4_add_plugin(kontact_specialdatesplugin WITH_PREFIX ${kontact_specialdatesplugin_PART_SRCS})



target_link_libraries(kontact_specialdatesplugin  ${KDE4_KDECORE_LIBS} kpinterfaces kdepim kholidays korganizer_calendar kaddressbook )

install(TARGETS kontact_specialdatesplugin  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kcm_sdsummary_PART_SRCS kcmsdsummary.cpp )


kde4_add_ui_files(kcm_sdsummary_PART_SRCS sdsummaryconfig_base.ui )

kde4_add_plugin(kcm_sdsummary ${kcm_sdsummary_PART_SRCS})



target_link_libraries(kcm_sdsummary  ${KDE4_KDEUI_LIBS} ${QT_QT3SUPPORT_LIBRARY} )

install(TARGETS kcm_sdsummary  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES specialdatesplugin.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kontact)
install( FILES kcmsdsummary.desktop  DESTINATION ${SERVICES_INSTALL_DIR})


