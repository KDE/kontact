INCLUDES = -I$(top_srcdir)/kontact/interfaces -I$(top_srcdir)/certmanager/lib -I$(top_srcdir)/libkdepim -I$(top_srcdir) $(all_includes)

kde_module_LTLIBRARIES = libkontact_summaryplugin.la kcm_kontactsummary.la
libkontact_summaryplugin_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkontact_summaryplugin_la_LIBADD = -lkutils \
  $(top_builddir)/kontact/interfaces/libkpinterfaces.la $(LIB_KPARTS) \
  $(top_builddir)/libkdepim/libkdepim.la $(top_builddir)/libkpimidentities/libkpimidentities.la

libkontact_summaryplugin_la_SOURCES = summaryview_plugin.cpp summaryview_part.cpp dropwidget.cpp

kcm_kontactsummary_la_SOURCES = kcmkontactsummary.cpp
kcm_kontactsummary_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) \
                                -avoid-version -no-undefined
kcm_kontactsummary_la_LIBADD = $(LIB_KDEUI) $(LIB_KUTILS)

METASOURCES = AUTO

servicedir = $(kde_servicesdir)/kontact
service_DATA = summaryplugin.desktop

kde_services_DATA = kcmkontactsummary.desktop

kpartrcdir = $(kde_datadir)/kontactsummary
kpartrc_DATA = kontactsummary_part.rc
