



########### next target ###############

set(kpinterfaces_LIB_SRCS 
   core.cpp 
   plugin.cpp 
   summary.cpp 
   uniqueapphandler.cpp)

kde4_automoc(${kpinterfaces_LIB_SRCS})

kde4_add_library(kpinterfaces SHARED ${kpinterfaces_LIB_SRCS})

target_link_libraries(kpinterfaces  ${KDE4_KPARTS_LIBS} kdepim)

set_target_properties(kpinterfaces PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS kpinterfaces  DESTINATION ${LIB_INSTALL_DIR})


########### install files ###############

install( FILES kontact_export.h core.h plugin.h summary.h  DESTINATION ${INCLUDE_INSTALL_DIR}/kontact)
install( FILES kontactplugin.desktop  DESTINATION ${SERVICETYPES_INSTALL_DIR})

