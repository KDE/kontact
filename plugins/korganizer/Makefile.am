# top_srcdir/korganizer is a hack, to get korganizeriface.h
INCLUDES = -I$(top_srcdir)/kontact/interfaces -I$(top_srcdir)/korganizer \
  -I$(top_srcdir) $(all_includes)

kde_module_LTLIBRARIES = libkontact_korganizerplugin.la libkontact_todoplugin.la

libkontact_korganizerplugin_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkontact_korganizerplugin_la_LIBADD = \
  $(top_builddir)/kontact/interfaces/libkpinterfaces.la $(LIB_KPARTS) $(top_builddir)/libkcal/libkcal.la
libkontact_korganizerplugin_la_SOURCES = korganizerplugin.cpp kcalendariface.stub \
  summarywidget.cpp

libkontact_todoplugin_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkontact_todoplugin_la_LIBADD = \
  $(top_builddir)/kontact/interfaces/libkpinterfaces.la $(LIB_KPARTS) $(top_builddir)/libkcal/libkcal.la
libkontact_todoplugin_la_SOURCES = todoplugin.cpp kcalendariface.stub

METASOURCES = AUTO

kcalendariface_DIR = $(top_srcdir)/korganizer

servicedir = $(kde_servicesdir)/kontact
service_DATA = korganizerplugin.desktop todoplugin.desktop

KDE_ICON = AUTO
