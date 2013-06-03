
add_subdirectory(about)

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/libkdepimdbusinterfaces 
  ${CMAKE_SOURCE_DIR}/libkdepim/
)

########### next target ###############

set(kontactprivate_LIB_SRCS
  mainwindow.cpp
  sidepanebase.cpp
  iconsidepane.cpp
  aboutdialog.cpp
  kontactconfiguredialog.cpp
)

kde4_add_kcfg_files(kontactprivate_LIB_SRCS prefs.kcfgc)

kde4_add_library(kontactprivate ${LIBRARY_TYPE} ${kontactprivate_LIB_SRCS})

target_link_libraries(kontactprivate
  ${KDE4_KCMUTILS_LIBS}
  ${KDE4_KDEWEBKIT_LIBRARY}
  ${KDE4_KPARTS_LIBS}
  ${KDEPIMLIBS_KONTACTINTERFACE_LIBS}
  ${KDEPIMLIBS_KPIMUTILS_LIBS}
  ${QT_QTWEBKIT_LIBRARY}
  kdepim
  kdepimdbusinterfaces
)

set_target_properties(kontactprivate PROPERTIES
  VERSION ${GENERIC_LIB_VERSION}
  SOVERSION ${GENERIC_LIB_SOVERSION}
)

install(TARGETS kontactprivate ${INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)

########### next target ###############

set(kontact_bin_SRCS main.cpp)

kde4_add_app_icon(kontact_bin_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/icons/hi*-apps-kontact.png")

kde4_add_executable(kontact_bin ${kontact_bin_SRCS})

set_target_properties(kontact_bin PROPERTIES OUTPUT_NAME kontact)

target_link_libraries(kontact_bin
  ${KDE4_KDEWEBKIT_LIBRARY}
  ${KDE4_KPARTS_LIBRARY}
  ${KDEPIMLIBS_KONTACTINTERFACE_LIBS}
  ${QT_QTWEBKIT_LIBRARY}
  kdepimdbusinterfaces
  kontactprivate
)

install(TARGETS kontact_bin ${INSTALL_TARGETS_DEFAULT_ARGS})

########### next target ###############

set(kcm_kontact_PART_SRCS kcmkontact.cpp)

kde4_add_plugin(kcm_kontact ${kcm_kontact_PART_SRCS})

target_link_libraries(kcm_kontact
  ${KDE4_KCMUTILS_LIBS}
  ${KDE4_KDEUI_LIBS}
  ${QT_QTGUI_LIBRARY}
  kdepim
  kontactprivate
)

install(TARGETS kcm_kontact DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install(PROGRAMS Kontact.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES kontact.kcfg DESTINATION ${KCFG_INSTALL_DIR})
install(FILES kontactconfig.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES kontactui.rc DESTINATION ${DATA_INSTALL_DIR}/kontact)

add_subdirectory(icons)

