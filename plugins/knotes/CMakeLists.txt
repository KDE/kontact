
########### next target ###############

set(kontact_knotesplugin_PART_SRCS ${libknotesresources_SRCS} ${libknoteseditor_SRCS}
  knotes_plugin.cpp
  knotes_part.cpp
  summarywidget.cpp
  knotetip.cpp
  ${CMAKE_SOURCE_DIR}/knotes/knoteprinter.cpp
)

qt4_add_dbus_adaptor(kontact_knotesplugin_PART_SRCS org.kde.kontact.KNotes.xml knotes_part.h KNotesPart)

kde4_add_plugin(kontact_knotesplugin ${kontact_knotesplugin_PART_SRCS})

target_link_libraries(kontact_knotesplugin ${KDEPIMLIBS_KONTACTINTERFACE_LIBS} ${KDEPIMLIBS_KCAL_LIBS} ${KDEPIMLIBS_KRESOURCES_LIBS} ${KDE4_KDEPRINT_LIBS} ${KDE4_KPARTS_LIBS} kdepim ${KDE4_KPRINTUTILS_LIBS})

########### install files ###############

install(TARGETS kontact_knotesplugin DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES knotes_part.rc DESTINATION ${DATA_INSTALL_DIR}/knotes)
install(FILES knotesplugin.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kontact)
install(FILES org.kde.kontact.KNotes.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})

install(FILES knotes.setdlg DESTINATION ${DATA_INSTALL_DIR}/kontact/ksettingsdialog)
