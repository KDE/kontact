INCLUDES = -I$(top_srcdir)/kontact/interfaces -I$(top_builddir)/libkdepim -I$(top_srcdir) $(all_includes)

kde_module_LTLIBRARIES = libkontact_weatherplugin.la
libkontact_weatherplugin_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkontact_weatherplugin_la_LIBADD = $(top_builddir)/kontact/interfaces/libkpinterfaces.la $(LIB_KPARTS) $(top_builddir)/libkdepim/libkdepim.la

libkontact_weatherplugin_la_SOURCES = weather_plugin.cpp summarywidget.cpp \
                                      summarywidget.skel

METASOURCES = AUTO

servicedir = $(kde_servicesdir)/kontact
service_DATA = weatherplugin.desktop
