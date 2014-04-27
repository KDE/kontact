
include_directories(${CMAKE_SOURCE_DIR}/korganizer ${CMAKE_SOURCE_DIR}/korganizer/interfaces ${Boost_INCLUDE_DIRS})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

########### next target ###############
set(libcommon_SRCS korg_uniqueapp.cpp)

set(kontact_korganizerplugin_PART_SRCS korganizerplugin.cpp apptsummarywidget.cpp summaryeventinfo.cpp ${libcommon_SRCS})

qt4_add_dbus_interfaces(kontact_korganizerplugin_PART_SRCS ${CMAKE_SOURCE_DIR}/korganizer/org.kde.Korganizer.Calendar.xml  ${CMAKE_SOURCE_DIR}/korganizer/org.kde.korganizer.Korganizer.xml)

kde4_add_plugin(kontact_korganizerplugin ${kontact_korganizerplugin_PART_SRCS})

target_link_libraries(kontact_korganizerplugin KF5::AkonadiCalendar ${KDEPIMLIBS_KCALUTILS_LIBS} ${KDE4_KPARTS_LIBS} ${KDEPIMLIBS_KABC_LIBS} ${KDEPIMLIBS_KCALCORE_LIBS} kdepim ${KDEPIMLIBS_KONTACTINTERFACE_LIBS} korganizerprivate calendarsupport akonadi-calendar)

########### next target ###############

set(kontact_todoplugin_PART_SRCS todoplugin.cpp todosummarywidget.cpp ${libcommon_SRCS})

qt4_add_dbus_interfaces(kontact_todoplugin_PART_SRCS ${CMAKE_SOURCE_DIR}/korganizer/org.kde.Korganizer.Calendar.xml  ${CMAKE_SOURCE_DIR}/korganizer/org.kde.korganizer.Korganizer.xml)

kde4_add_plugin(kontact_todoplugin ${kontact_todoplugin_PART_SRCS})

target_link_libraries(kontact_todoplugin KF5::AkonadiCalendar ${KDE4_KPARTS_LIBS} ${KDEPIMLIBS_KABC_LIBS} kdepim ${KDEPIMLIBS_KONTACTINTERFACE_LIBS} ${KDEPIMLIBS_KCALCORE_LIBS} ${KDEPIMLIBS_KCALUTILS_LIBS} korganizerprivate calendarsupport akonadi-calendar)

########### next target ###############

set(kontact_journalplugin_PART_SRCS journalplugin.cpp ${libcommon_SRCS})

qt4_add_dbus_interfaces(kontact_journalplugin_PART_SRCS ${CMAKE_SOURCE_DIR}/korganizer/org.kde.Korganizer.Calendar.xml)

kde4_add_plugin(kontact_journalplugin ${kontact_journalplugin_PART_SRCS})

target_link_libraries(kontact_journalplugin ${KDE4_KPARTS_LIBS} ${KDEPIMLIBS_KONTACTINTERFACE_LIBS})

########### next target ###############

set(kcm_apptsummary_PART_SRCS kcmapptsummary.cpp)

kde4_add_ui_files(kcm_apptsummary_PART_SRCS apptsummaryconfig_base.ui)

kde4_add_plugin(kcm_apptsummary ${kcm_apptsummary_PART_SRCS})

target_link_libraries(kcm_apptsummary ${KDE4_KDEUI_LIBS} korganizerprivate ${KDEPIMLIBS_KCALUTILS_LIBS})

########### next target ###############

set(kcm_todosummary_PART_SRCS kcmtodosummary.cpp)

kde4_add_ui_files(kcm_todosummary_PART_SRCS todosummaryconfig_base.ui)

kde4_add_plugin(kcm_todosummary ${kcm_todosummary_PART_SRCS})

target_link_libraries(kcm_todosummary ${KDE4_KDEUI_LIBS})

########## Unit Test ###########
add_subdirectory(tests)

########### install files ###############

install(TARGETS kcm_apptsummary DESTINATION ${PLUGIN_INSTALL_DIR})
install(TARGETS kcm_todosummary DESTINATION ${PLUGIN_INSTALL_DIR})
install(TARGETS kontact_korganizerplugin DESTINATION ${PLUGIN_INSTALL_DIR})
install(TARGETS kontact_todoplugin DESTINATION ${PLUGIN_INSTALL_DIR})
install(TARGETS kontact_journalplugin DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES korganizerplugin.desktop todoplugin.desktop journalplugin.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kontact)
install(FILES kcmapptsummary.desktop kcmtodosummary.desktop DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES korganizer.setdlg DESTINATION ${DATA_INSTALL_DIR}/kontact/ksettingsdialog)

kde4_install_icons(${ICON_INSTALL_DIR})


