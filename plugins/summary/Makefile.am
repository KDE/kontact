INCLUDES = -I$(top_srcdir)/kontact/interfaces -I$(top_srcdir)/libkdepim $(all_includes)

kde_module_LTLIBRARIES = libkpsummaryplugin.la
libkpsummaryplugin_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkpsummaryplugin_la_LIBADD = $(top_builddir)/kontact/interfaces/libkpinterfaces.la $(LIB_KPARTS) $(top_builddir)/libkdepim/libkdepim.la

libkpsummaryplugin_la_SOURCES = summaryview_plugin.cpp summaryview_part.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kpsummaryplugin.desktop

rc_DATA = kpsummaryplugin.rc
rcdir = $(kde_datadir)/kpsummaryplugin

kpartrc_DATA = summarypartui.rc
kpartrcdir = $(kde_datadir)/summaryviewpart
