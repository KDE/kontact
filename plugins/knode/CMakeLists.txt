
include_directories( ${CMAKE_SOURCE_DIR}/kontact/interfaces ${CMAKE_SOURCE_DIR}/knode  )


########### next target ###############

set(kontact_knodeplugin_PART_SRCS knode_plugin.cpp )

kde4_automoc(${kontact_knodeplugin_PART_SRCS})

#kde4_add_dcop_stubs(kontact_knodeplugin_PART_SRCS ${CMAKE_SOURCE_DIR}/knode/knodeiface.h )

kde4_add_plugin(kontact_knodeplugin WITH_PREFIX ${kontact_knodeplugin_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kontact_knodeplugin )

target_link_libraries(kontact_knodeplugin ${KDE4_KDECORE_LIBS} ${KDE4_KPARTS_LIBS} kpinterfaces)

install(TARGETS kontact_knodeplugin  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES knodeplugin.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kontact)




#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/kontact/interfaces -I$(top_srcdir)/knode -I$(top_srcdir) $(all_includes)
#
#kde_module_LTLIBRARIES = libkontact_knodeplugin.la
#libkontact_knodeplugin_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkontact_knodeplugin_la_LIBADD = $(top_builddir)/kontact/interfaces/libkpinterfaces.la $(LIB_KPARTS)
#
#libkontact_knodeplugin_la_SOURCES = knode_plugin.cpp knodeiface.stub
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)/kontact
#service_DATA = knodeplugin.desktop
#
#knodeiface_DIR = $(top_srcdir)/knode
