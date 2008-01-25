add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

include_directories( ${CMAKE_SOURCE_DIR}/kontact/interfaces ${CMAKE_SOURCE_DIR}/libkdepim  )


########### next target ###############

set(kontact_plannerplugin_PART_SRCS plannerplugin.cpp planner.cpp )

qt4_add_dbus_interfaces(kontact_plannerplugin_PART_SRCS ${CMAKE_SOURCE_DIR}/kaddressbook/org.kde.KAddressbook.Core.xml )

kde4_add_plugin(kontact_plannerplugin ${kontact_plannerplugin_PART_SRCS})

kdepim4_link_unique_libraries(kontact_plannerplugin  ${KDE4_KDECORE_LIBS} kpinterfaces kdepim kholidays korganizer_calendar kaddressbook )

install(TARGETS kontact_plannerplugin  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kcm_planner_PART_SRCS kcmplanner.cpp )

kde4_add_plugin(kcm_planner ${kcm_planner_PART_SRCS})

kdepim4_link_unique_libraries(kcm_planner ${KDE4_KDEUI_LIBS} ${QT_QT3SUPPORT_LIBRARY} )

install(TARGETS kcm_planner DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES plannerplugin.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kontact)
install( FILES kcmplanner.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

