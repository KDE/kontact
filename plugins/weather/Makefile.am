INCLUDES = -I$(top_srcdir)/kontact/interfaces -I$(top_srcdir)/libkdepim $(all_includes)

kde_module_LTLIBRARIES = libkpweatherplugin.la
libkpweatherplugin_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkpweatherplugin_la_LIBADD = $(top_builddir)/kontact/interfaces/libkpinterfaces.la $(LIB_KPARTS) $(top_builddir)/libkdepim/libkdepim.la

libkpweatherplugin_la_SOURCES = weather_plugin.cpp summarywidget.cpp \
                                summarywidget.skel

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kpweatherplugin.desktop

