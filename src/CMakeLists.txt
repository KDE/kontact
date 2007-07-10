
add_subdirectory( about ) 

include_directories( ${CMAKE_SOURCE_DIR}/kontact/interfaces  )


########### next target ###############

set(kontact_LIB_SRCS 
   mainwindow.cpp 
   sidepanebase.cpp 
   iconsidepane.cpp 
   aboutdialog.cpp )


kde4_add_kcfg_files(kontact_LIB_SRCS prefs.kcfgc )

kde4_add_library(kontact SHARED ${kontact_LIB_SRCS})

target_link_libraries(kontact  ${KDE4_KPARTS_LIBS} kdepim kpinterfaces ${KDE4_KUTILS_LIBRARY} ${KDE4_KHTML_LIBRARY} )

set_target_properties(kontact PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kontact  DESTINATION ${LIB_INSTALL_DIR})


########### next target ###############

set(kontact_bin_SRCS main.cpp )


kde4_add_executable(kontact_bin ${kontact_bin_SRCS})

set_target_properties(kontact_bin  PROPERTIES OUTPUT_NAME kontact)

target_link_libraries(kontact_bin  ${KDE4_KHTML_LIBS} ${KDE4_KDE3SUPPORT_LIBS} kontact)

install(TARGETS kontact_bin  DESTINATION ${BIN_INSTALL_DIR} )


########### next target ###############

set(kcm_kontact_PART_SRCS kcmkontact.cpp )


kde4_add_plugin(kcm_kontact ${kcm_kontact_PART_SRCS})



target_link_libraries(kcm_kontact  ${KDE4_KDECORE_LIBS} kontact kdepim )

install(TARGETS kcm_kontact  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES Kontact.desktop  DESTINATION ${XDG_APPS_DIR})
install( FILES kontact.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install( FILES kontactconfig.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES kontactui.rc  kontact.setdlg DESTINATION ${DATA_INSTALL_DIR}/kontact)

kde4_install_icons( ${ICON_INSTALL_DIR}   )

