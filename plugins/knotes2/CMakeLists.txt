
########### next target ###############
include_directories(
    ${CMAKE_SOURCE_DIR}/pimcommon/
    ${CMAKE_SOURCE_DIR}/mailcommon
    ${CMAKE_SOURCE_DIR}/noteshared
    ${CMAKE_BINARY_DIR}/noteshared
    ${CMAKE_SOURCE_DIR}/knotes/
    ${CMAKE_BINARY_DIR}/knotes/
    ${CMAKE_SOURCE_DIR}/knotes2/
    ${CMAKE_BINARY_DIR}/knotes2/
    ../knotes/

    ${Grantlee_INCLUDE_DIRS}
)

set(kontact_knotes2plugin_PART_SRCS 
  knotes_plugin.cpp
  knotes_part.cpp
  summarywidget.cpp
  knotetip.cpp
  knotesiconview.cpp
  knoteslistwidgetsearchline.cpp
  knoteswidget.cpp
  ../knotes/knoteseditdialog.cpp  
  knotesselectdeletenotesdialog.cpp
)

qt4_add_dbus_adaptor(kontact_knotes2plugin_PART_SRCS ../knotes/org.kde.kontact.KNotes.xml ../knotes/knotes_part.h KNotesPart)

kde4_add_plugin(kontact_knotes2plugin ${kontact_knotes2plugin_PART_SRCS})

target_link_libraries(kontact_knotes2plugin knotesprivate ${KDEPIMLIBS_KONTACTINTERFACE_LIBS} ${KDEPIMLIBS_KCALCORE_LIBS} ${KDEPIMLIBS_KCALUTILS_LIBS} ${KDE4_KDEPRINT_LIBS} ${KDE4_KPARTS_LIBS} kdepim ${KDE4_KPRINTUTILS_LIBS} pimcommon ${KDE4_KDNSSD_LIBS} noteshared)

install(TARGETS kontact_knotes2plugin DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kcm_knotessummary_PART_SRCS kcmknotessummary.cpp )

kde4_add_plugin(kcm_knotessummary ${kcm_knotessummary_PART_SRCS})
#add_dependencies(kcm_knotessummary kmail_xml)

target_link_libraries(kcm_knotessummary ${KDE4_KDEUI_LIBS} ${KDEPIMLIBS_KMIME_LIBS} ${KDEPIMLIBS_AKONADI_LIBS} akonadi_next pimcommon)

########### install files ###############

install(TARGETS kcm_knotessummary DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES kcmknotessummary.desktop DESTINATION ${SERVICES_INSTALL_DIR})


#install(FILES knotes_part.rc DESTINATION ${DATA_INSTALL_DIR}/knotes)
install(FILES knotes2plugin.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kontact)
#install(FILES org.kde.kontact.KNotes.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})

#install(FILES knotes.setdlg DESTINATION ${DATA_INSTALL_DIR}/kontact/ksettingsdialog)
