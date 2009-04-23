
########### next target ###############

set(kontact_specialdatesplugin_PART_SRCS specialdates_plugin.cpp sdsummarywidget.cpp)

qt4_add_dbus_interfaces(kontact_specialdatesplugin_PART_SRCS ${CMAKE_SOURCE_DIR}/kaddressbook/org.kde.KAddressbook.Core.xml)

kde4_add_plugin(kontact_specialdatesplugin ${kontact_specialdatesplugin_PART_SRCS})

kdepim4_link_unique_libraries(kontact_specialdatesplugin  ${KDE4_KDEUI_LIBS} ${KDE4_KCAL_LIBS} ${KDE4_KABC_LIBS} ${KDE4_KRESOURCES_LIBS} kontactinterfaces kdepim kholidays korganizer_calendar kaddressbookprivate)


########### next target ###############

set(kcm_sdsummary_PART_SRCS kcmsdsummary.cpp)

kde4_add_ui_files(kcm_sdsummary_PART_SRCS sdsummaryconfig_base.ui)

kde4_add_plugin(kcm_sdsummary ${kcm_sdsummary_PART_SRCS})

kdepim4_link_unique_libraries(kcm_sdsummary  ${KDE4_KDEUI_LIBS} ${QT_QT3SUPPORT_LIBRARY})

########### install files ###############

install(TARGETS kontact_specialdatesplugin  DESTINATION ${PLUGIN_INSTALL_DIR})
install(TARGETS kcm_sdsummary  DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES specialdatesplugin.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kontact)
install(FILES kcmsdsummary.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES specialdates.setdlg DESTINATION ${DATA_INSTALL_DIR}/kontact/ksettingsdialog)

