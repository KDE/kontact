INCLUDES = -I$(top_srcdir)/kontact/interfaces $(all_includes)

kde_module_LTLIBRARIES = libkpkaddressbookplugin.la
libkpkaddressbookplugin_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -lkabc
libkpkaddressbookplugin_la_LIBADD = $(top_builddir)/kontact/interfaces/libkpinterfaces.la 

libkpkaddressbookplugin_la_SOURCES = kaddressbook_plugin.cpp kaddressbookiface.stub \
                                     kabsummarywidget.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kpkaddressbookplugin.desktop

kaddressbookiface_DIR = $(top_srcdir)/kaddressbook

rc_DATA = kpkaddressbookplugin.rc
rcdir = $(kde_datadir)/kpkaddressbookplugin
