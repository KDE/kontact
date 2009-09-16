add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

include_directories(${CMAKE_SOURCE_DIR}/korganizer ${CMAKE_SOURCE_DIR}/korganizer/interfaces)

########### next target ###############

set(kontact_plannerplugin_PART_SRCS plannerplugin.cpp planner.cpp)

qt4_add_dbus_interfaces(kontact_plannerplugin_PART_SRCS ${CMAKE_SOURCE_DIR}/korganizer/org.kde.Korganizer.Calendar.xml  ${CMAKE_SOURCE_DIR}/korganizer/org.kde.korganizer.Korganizer.xml)

kde4_add_plugin(kontact_plannerplugin ${kontact_plannerplugin_PART_SRCS})

target_link_libraries(kontact_plannerplugin  ${KDE4_KDECORE_LIBS} ${KDEPIMLIBS_KCAL_LIBS} ${KDEPIMLIBS_KHOLIDAYS_LIBS} kontactinterfaces kdepim korganizerprivate kaddressbookprivate)

########### next target ###############

set(kcm_planner_PART_SRCS kcmplanner.cpp)

kde4_add_ui_files(kcm_planner_PART_SRCS plannerconfig_base.ui)

kde4_add_plugin(kcm_planner ${kcm_planner_PART_SRCS})

target_link_libraries(kcm_planner
  ${KDE4_KDEUI_LIBS}
  ${QT_QT3SUPPORT_LIBRARY}
  ${KDEPIMLIBS_KHOLIDAYS_LIBS}
)

########### install files ###############

install(TARGETS kontact_plannerplugin  DESTINATION ${PLUGIN_INSTALL_DIR})
install(TARGETS kcm_planner DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES plannerplugin.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kontact)
install(FILES kcmplanner.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES planner.setdlg DESTINATION ${DATA_INSTALL_DIR}/kontact/ksettingsdialog)
