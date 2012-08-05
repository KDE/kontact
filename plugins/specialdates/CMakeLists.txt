
include_directories(${Boost_INCLUDE_DIRS})

########### next target ###############

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

include_directories(${Boost_INCLUDE_DIR})

set(kontact_specialdatesplugin_PART_SRCS specialdates_plugin.cpp sdsummarywidget.cpp)

kde4_add_plugin(kontact_specialdatesplugin ${kontact_specialdatesplugin_PART_SRCS})

if(BUILD_korganizer)
  set(_korganizerprivate_lib "korganizerprivate")
endif()

target_link_libraries(kontact_specialdatesplugin
  akonadi-calendar
  ${KDE4_KDEUI_LIBS}
  ${KDEPIMLIBS_KABC_LIBS}
  ${KDEPIMLIBS_KCALCORE_LIBS}
  ${KDEPIMLIBS_KHOLIDAYS_LIBS}
  ${KDEPIMLIBS_KONTACTINTERFACE_LIBS}
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_AKONADI_CONTACT_LIBS}
  kdepim
  calendarsupport
  ${_korganizerprivate_lib}
)

########### next target ###############

set(kcm_sdsummary_PART_SRCS kcmsdsummary.cpp)

kde4_add_ui_files(kcm_sdsummary_PART_SRCS sdsummaryconfig_base.ui)

kde4_add_plugin(kcm_sdsummary ${kcm_sdsummary_PART_SRCS})

target_link_libraries(kcm_sdsummary ${KDE4_KDEUI_LIBS})

########### install files ###############

install(TARGETS kontact_specialdatesplugin DESTINATION ${PLUGIN_INSTALL_DIR})
install(TARGETS kcm_sdsummary DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES specialdatesplugin.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kontact)
install(FILES kcmsdsummary.desktop DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES specialdates.setdlg DESTINATION ${DATA_INSTALL_DIR}/kontact/ksettingsdialog)

