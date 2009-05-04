
add_subdirectory( about ) 

include_directories( ${CMAKE_SOURCE_DIR}/kontactinterfaces  )


########### next target ###############

set(kontactprivate_LIB_SRCS 
   mainwindow.cpp 
   sidepanebase.cpp 
   iconsidepane.cpp 
   aboutdialog.cpp 
   profiledialog.cpp 
   profilemanager.cpp )


kde4_add_kcfg_files(kontactprivate_LIB_SRCS prefs.kcfgc )

kde4_add_library(kontactprivate SHARED ${kontactprivate_LIB_SRCS})

kdepim4_link_unique_libraries(kontactprivate  ${KDE4_KPARTS_LIBS} kdepim kontactinterfaces ${KDE4_KUTILS_LIBS} ${KDE4_KHTML_LIBRARY} ${KDE4_KPIMUTILS_LIBS} ${KDE4_KDE3SUPPORT_LIBRARY} ${QT_QT3SUPPORT_LIBRARY})

set_target_properties(kontactprivate PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kontactprivate  DESTINATION ${LIB_INSTALL_DIR})


########### next target ###############

set(kontact_bin_SRCS main.cpp )

kde4_add_app_icon(kontact_bin_SRCS "${KDE4_ICON_DIR}/oxygen/*/apps/kontact.png")

kde4_add_executable(kontact_bin ${kontact_bin_SRCS})

set_target_properties(kontact_bin  PROPERTIES OUTPUT_NAME kontact)

kdepim4_link_unique_libraries(kontact_bin  ${KDE4_KHTML_LIBS} ${KDE4_KDE3SUPPORT_LIBS} kontactprivate kontactinterfaces kdepim)


install(TARGETS kontact_bin  ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### next target ###############

set(kcm_kontact_PART_SRCS kcmkontact.cpp )


kde4_add_plugin(kcm_kontact ${kcm_kontact_PART_SRCS})



kdepim4_link_unique_libraries(kcm_kontact  ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} ${KDE4_KUTILS_LIBS} kontactprivate kdepim ${QT_QT3SUPPORT_LIBRARY})

install(TARGETS kcm_kontact  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES Kontact.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
install( FILES kontact.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install( FILES kontactconfig.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES kontactui.rc DESTINATION ${DATA_INSTALL_DIR}/kontact)

kde4_install_icons( ${ICON_INSTALL_DIR}   )

