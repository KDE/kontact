INCLUDES = -I$(top_srcdir)/kontact/interfaces -I$(top_srcdir)/libkdepim $(all_includes)

kde_module_LTLIBRARIES = libkontact_summaryplugin.la
libkontact_summaryplugin_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkontact_summaryplugin_la_LIBADD = -lkutils \
  $(top_builddir)/kontact/interfaces/libkpinterfaces.la $(LIB_KPARTS) \
  $(top_builddir)/libkdepim/libkdepim.la

libkontact_summaryplugin_la_SOURCES = summaryview_plugin.cpp summaryview_part.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)/kontact
service_DATA = summaryplugin.desktop

kpartrcdir = $(kde_datadir)/kontactsummary
kpartrc_DATA = kontactsummary_part.rc
