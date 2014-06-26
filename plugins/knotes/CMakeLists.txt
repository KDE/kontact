


########### next target ###############
include_directories(
    ${CMAKE_SOURCE_DIR}/pimcommon/
    ${CMAKE_SOURCE_DIR}/noteshared
    ${CMAKE_BINARY_DIR}/noteshared
    ${CMAKE_SOURCE_DIR}/knotes/
    ${CMAKE_BINARY_DIR}/knotes/
    ${Grantlee_INCLUDE_DIRS}
)

set(kontact_knotesplugin_PART_SRCS 
  knotes_plugin.cpp
  knotes_part.cpp
  summarywidget.cpp
  knotesiconview.cpp
  knoteslistwidgetsearchline.cpp
  knoteswidget.cpp
  knoteseditdialog.cpp  
  knotesselectdeletenotesdialog.cpp
)

qt5_add_dbus_adaptor(kontact_knotesplugin_PART_SRCS org.kde.kontact.KNotes.xml knotes_part.h KNotesPart)
qt5_add_dbus_interfaces(kontact_knotesplugin_interface_SRCS org.kde.kontact.KNotes.xml)



add_library(kontact_knotesplugin MODULE ${kontact_knotesplugin_PART_SRCS} ${kontact_knotesplugin_interface_SRCS} )

target_link_libraries(kontact_knotesplugin 
                      knotesprivate 
                      KF5::CalendarCore 
                      KF5::CalendarUtils 
                      KF5::KontactInterface 
                      kdepim 
                      pimcommon 
                      noteshared 
                      akonadi_next
)

########### install files ###############

set(kcm_knotessummary_PART_SRCS kcmknotessummary.cpp )

add_library(kcm_knotessummary MODULE ${kcm_knotessummary_PART_SRCS})

target_link_libraries(kcm_knotessummary  KF5::Mime KF5::AkonadiCore akonadi_next pimcommon)


install(TARGETS kcm_knotessummary DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES kcmknotessummary.desktop DESTINATION ${SERVICES_INSTALL_DIR})


install(TARGETS kontact_knotesplugin DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES knotes_part.rc DESTINATION ${DATA_INSTALL_DIR}/knotes)
install(FILES knotesplugin.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kontact)
install(FILES org.kde.kontact.KNotes.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})

install(FILES knotes.setdlg DESTINATION ${DATA_INSTALL_DIR}/kontact/ksettingsdialog)
