INCLUDES = -I$(top_srcdir)/kaplan/interfaces $(all_includes)

kde_module_LTLIBRARIES = libkpkaddressbookplugin.la
libkpkaddressbookplugin_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkpkaddressbookplugin_la_LIBADD = $(top_builddir)/kaplan/interfaces/libkpinterfaces.la $(LIB_KPARTS)

libkpkaddressbookplugin_la_SOURCES = kaddressbook_plugin.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kpkaddressbookplugin.desktop

rc_DATA = kpkaddressbookplugin.rc
rcdir = $(kde_datadir)/kpkaddressbookplugin
