INCLUDES = -I$(top_srcdir)/kaplan/interfaces $(all_includes)

kde_module_LTLIBRARIES = libkpkaddressbookplugin.la
libkpkaddressbookplugin_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkpkaddressbookplugin_la_LIBADD = $(top_builddir)/kaplan/interfaces/libkpinterfaces.la 

libkpkaddressbookplugin_la_SOURCES = kaddressbook_plugin.cpp kaddressbookiface_stub.h

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kpkaddressbookplugin.desktop

kaddressbookiface_DIR = $(top_srcdir)/kaddressbook

rc_DATA = kpkaddressbookplugin.rc
rcdir = $(kde_datadir)/kpkaddressbookplugin
