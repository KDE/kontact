
include_directories( ${CMAKE_SOURCE_DIR}/kontact/interfaces  )


########### next target ###############

set(kontact_knotesplugin_PART_SRCS ${libknotesresources_SRCS} ${libknoteseditor_SRCS} 
   knotes_plugin.cpp 
   knotes_part.cpp 
   summarywidget.cpp 
   knotetip.cpp )

qt4_add_dbus_adaptor(kontact_knotesplugin_PART_SRCS org.kde.kontact.KNotes.xml knotes_part.h KNotesPart  )

kde4_automoc(${kontact_knotesplugin_PART_SRCS})

kde4_add_plugin(kontact_knotesplugin WITH_PREFIX ${kontact_knotesplugin_PART_SRCS})



target_link_libraries(kontact_knotesplugin  ${KDE4_KDECORE_LIBS} ${KDE4_KPARTS_LIBS} kpinterfaces kdepim ${KDE4_KCAL_LIBS} ${KDE4_KRESOURCES_LIBS})

install(TARGETS kontact_knotesplugin  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES knotes_part.rc  DESTINATION ${DATA_INSTALL_DIR}/knotes)
install( FILES knotesplugin.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kontact)
install( FILES org.kde.kontact.KNotes.xml DESTINATION ${DBUS_INTERFACES_DIR})


