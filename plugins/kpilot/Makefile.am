INCLUDES = -I$(top_srcdir)/kontact/interfaces -I$(top_srcdir)/libkdepim -I$(top_srcdir)/kpilot/lib -I$(top_srcdir)/kpilot/kpilot $(all_includes)

kde_module_LTLIBRARIES = libkontact_kpilotplugin.la
libkontact_kpilotplugin_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkontact_kpilotplugin_la_LIBADD = $(top_builddir)/kontact/interfaces/libkpinterfaces.la $(LIB_KPARTS) $(top_builddir)/libkdepim/libkdepim.la $(top_builddir)/kpilot/lib/libkpilot.la 

libkontact_kpilotplugin_la_SOURCES = kpilot_plugin.cpp summarywidget.cpp \
                                      summarywidget.skel pilotDaemonDCOP.stub
																			
pilotDaemonDCOP_DIR = $(top_srcdir)/kpilot/kpilot

METASOURCES = AUTO

servicedir = $(kde_servicesdir)/kontact
service_DATA = kpilotplugin.desktop
