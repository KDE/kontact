
include_directories(${Boost_INCLUDE_DIRS})

########### next target ###############

kde_enable_exceptions()

include_directories(${Boost_INCLUDE_DIR})

set(kontact_specialdatesplugin_PART_SRCS specialdates_plugin.cpp sdsummarywidget.cpp)

add_library(kontact_specialdatesplugin MODULE ${kontact_specialdatesplugin_PART_SRCS})

set(_korganizerprivate_lib "korganizerprivate")

target_link_libraries(kontact_specialdatesplugin
  KF5::AkonadiCalendar
  
  KF5::Abc
  KF5::CalendarCore
  KF5::Holidays
  KF5::KontactInterface
  KF5::AkonadiCore
  KF5::AkonadiContact
  kdepim
  calendarsupport
  ${_korganizerprivate_lib}
)

########### next target ###############

set(kcm_sdsummary_PART_SRCS kcmsdsummary.cpp)

qt5_wrap_ui(kcm_sdsummary_PART_SRCS sdsummaryconfig_base.ui)

add_library(kcm_sdsummary MODULE ${kcm_sdsummary_PART_SRCS})

target_link_libraries(kcm_sdsummary Qt5::Widgets KF5::KCMUtils KF5::KDELibs4Support)

########### install files ###############

install(TARGETS kontact_specialdatesplugin DESTINATION ${PLUGIN_INSTALL_DIR})
install(TARGETS kcm_sdsummary DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES specialdatesplugin.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kontact)
install(FILES kcmsdsummary.desktop DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES specialdates.setdlg DESTINATION ${DATA_INSTALL_DIR}/kontact/ksettingsdialog)

