INCLUDES = -I$(top_srcdir)/kontact/interfaces -I$(top_srcdir)/kmail $(all_includes)

kde_module_LTLIBRARIES = libkpkmailplugin.la
libkpkmailplugin_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkpkmailplugin_la_LIBADD = $(top_builddir)/kontact/interfaces/libkpinterfaces.la $(LIB_KPARTS)

libkpkmailplugin_la_SOURCES = kmail_plugin.cpp kmailIface.stub

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kpkmailplugin.desktop

kmailIface_DIR = $(top_srcdir)/kmail

rc_DATA = kpkmailplugin.rc
rcdir = $(kde_datadir)/kpkmailplugin
