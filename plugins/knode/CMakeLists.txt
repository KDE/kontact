kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kontact/interfaces ${CMAKE_SOURCE_DIR}/knode ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kontact_knodeplugin_PART_SRCS knode_plugin.cpp )

kde4_automoc(${kontact_knodeplugin_PART_SRCS})

kde4_add_dcop_stubs(kontact_knodeplugin_PART_SRCS knodeiface.h )

kde4_add_plugin(kontact_knodeplugin WITH_PREFIX ${kontact_knodeplugin_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kontact_knodeplugin )

target_link_libraries(kontact_knodeplugin  ${QT_AND_KDECORE_LIBS} kpinterfaces kparts )

install_targets(${LIB_INSTALL_DIR}/kde4 kontact_knodeplugin )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR}/kontact FILES knodeplugin.desktop )

kde4_footer()



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
