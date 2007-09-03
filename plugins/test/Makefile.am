INCLUDES = -I$(top_srcdir)/kontact/interfaces $(all_includes)

kde_module_LTLIBRARIES = libkptestplugin.la
libkptestplugin_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkptestplugin_la_LIBADD = $(top_builddir)/kontact/interfaces/libkpinterfaces.la $(LIB_KPARTS)

libkptestplugin_la_SOURCES = test_plugin.cpp test_part.cpp kaddressbookiface.stub

kaddressbookiface_DIR = $(top_srcdir)/kaddressbook

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kptestplugin.desktop

rc_DATA = kptestplugin.rc
rcdir = $(kde_datadir)/kptestplugin

kpartrc_DATA = testpartui.rc
kpartrcdir = $(kde_datadir)/testpart
