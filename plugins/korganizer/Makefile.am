# top_srcdir/korganizer is a hack, to get korganizeriface.h
INCLUDES = -I$(top_srcdir)/kaplan/interfaces -I$(top_srcdir)/korganizer $(all_includes)

kde_module_LTLIBRARIES = libkpkorganizerplugin.la
libkpkorganizerplugin_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkpkorganizerplugin_la_LIBADD = $(top_builddir)/kaplan/interfaces/libkpinterfaces.la $(LIB_KPARTS)

libkpkorganizerplugin_la_SOURCES = korganizer_plugin.cpp korganizeriface.stub

METASOURCES = AUTO

korganizeriface_DIR = $(top_srcdir)/korganizer


servicedir = $(kde_servicesdir)
service_DATA = kpkorganizerplugin.desktop

rc_DATA = kpkorganizerplugin.rc
rcdir = $(kde_datadir)/kpkorganizerplugin
