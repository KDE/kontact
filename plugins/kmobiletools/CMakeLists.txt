
include_directories(  ${CMAKE_SOURCE_DIR}/kontact/interfaces ${CMAKE_SOURCE_DIR}/kmobiletools/src )


########### next target ###############

set(kontact_kmobiletools_PART_SRCS kmobiletools_plugin.cpp )

kde4_automoc(${kontact_kmobiletools_PART_SRCS})

kde4_add_plugin(kontact_kmobiletools WITH_PREFIX ${kontact_kmobiletools_PART_SRCS})

target_link_libraries(kontact_kmobiletools  ${KDE4_KDECORE_LIBS} kmobiletools kpinterfaces )

install(TARGETS kontact_kmobiletools DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES  kmobiletools.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kontact )

