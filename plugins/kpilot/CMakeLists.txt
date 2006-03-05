kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kontact/interfaces ${CMAKE_SOURCE_DIR}/libkdepim ${CMAKE_SOURCE_DIR}/kpilot/lib ${CMAKE_SOURCE_DIR}/kpilot/kpilot ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kontact_kpilotplugin_PART_SRCS kpilot_plugin.cpp summarywidget.cpp )

kde4_automoc(${kontact_kpilotplugin_PART_SRCS})

kde4_add_dcop_skels(kontact_kpilotplugin_PART_SRCS summarywidget.h )

kde4_add_dcop_stubs(kontact_kpilotplugin_PART_SRCS pilotDaemonDCOP.h )

kde4_add_plugin(kontact_kpilotplugin WITH_PREFIX ${kontact_kpilotplugin_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kontact_kpilotplugin )

target_link_libraries(kontact_kpilotplugin  ${QT_AND_KDECORE_LIBS} kpinterfaces kparts kdepim kpilot )

install_targets(${LIB_INSTALL_DIR}/kde4 kontact_kpilotplugin )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR}/kontact FILES kpilotplugin.desktop )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/kontact/interfaces \
#	-I$(top_srcdir)/libkdepim \
#	-I$(top_srcdir) \
#	-I$(top_srcdir)/kpilot/lib \
#	-I$(top_srcdir)/kpilot/kpilot \
#	$(PISOCK_INCLUDE) $(all_includes)
#
#kde_module_LTLIBRARIES = libkontact_kpilotplugin.la
#libkontact_kpilotplugin_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkontact_kpilotplugin_la_LIBADD = $(top_builddir)/kontact/interfaces/libkpinterfaces.la \
#	$(LIB_KPARTS) \
#	$(top_builddir)/libkdepim/libkdepim.la \
#	$(top_builddir)/kpilot/lib/libkpilot.la
#
#libkontact_kpilotplugin_la_SOURCES = kpilot_plugin.cpp summarywidget.cpp \
#                                      summarywidget.skel pilotDaemonDCOP.stub
#
#pilotDaemonDCOP_DIR = $(top_srcdir)/kpilot/kpilot
#pilotDaemonDCOP_DCOPIDLNG = true
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)/kontact
#service_DATA = kpilotplugin.desktop
