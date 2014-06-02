
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

kconfig_add_kcfg_files(kontactprivate_LIB_SRCS prefs.kcfgc)

add_library(kontactprivate ${LIBRARY_TYPE} ${kontactprivate_LIB_SRCS})

target_link_libraries(kontactprivate
  KF5::KCMUtils
  KF5::WebKit
  KF5::KontactInterface
  KF5::PimUtils
  kdepim
  kdepimdbusinterfaces
)

set_target_properties(kontactprivate PROPERTIES
  VERSION ${KDEPIM_LIB_VERSION}
  SOVERSION ${KDEPIM_LIB_SOVERSION}
)

install(TARGETS kontactprivate ${INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)

########### next target ###############

set(kontact_bin_SRCS main.cpp)

kde4_add_app_icon(kontact_bin_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/icons/hi*-apps-kontact.png")

add_executable(kontact_bin ${kontact_bin_SRCS})

set_target_properties(kontact_bin PROPERTIES OUTPUT_NAME kontact)

target_link_libraries(kontact_bin
  KF5::WebKit
  KF5::Service
  KF5::KontactInterface
  kdepimdbusinterfaces
  kontactprivate
)

install(TARGETS kontact_bin ${INSTALL_TARGETS_DEFAULT_ARGS})

########### next target ###############

set(kcm_kontact_PART_SRCS kcmkontact.cpp)

add_library(kcm_kontact MODULE ${kcm_kontact_PART_SRCS})

target_link_libraries(kcm_kontact
  KF5::KCMUtils
  
  
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

