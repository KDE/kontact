# top_srcdir/korganizer is a hack, to get korganizeriface.h
INCLUDES = -I$(top_srcdir)/kontact/interfaces -I$(top_srcdir)/korganizer \
  -I$(top_srcdir) $(all_includes)

kde_module_LTLIBRARIES = libkpkorganizerplugin.la
libkpkorganizerplugin_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkpkorganizerplugin_la_LIBADD = $(top_builddir)/kontact/interfaces/libkpinterfaces.la $(LIB_KPARTS) -L$(top_builddir)/libkcal -lkcal

libkpkorganizerplugin_la_SOURCES = korganizer_plugin.cpp kcalendariface.stub \
  summarywidget.cpp

METASOURCES = AUTO

kcalendariface_DIR = $(top_srcdir)/korganizer


servicedir = $(kde_servicesdir)
service_DATA = kpkorganizerplugin.desktop

rc_DATA = kpkorganizerplugin.rc
rcdir = $(kde_datadir)/kpkorganizerplugin
