INCLUDES = -I$(top_srcdir)/kontact/interfaces -I$(top_srcdir)/kmail $(all_includes)

kde_module_LTLIBRARIES = libkontact_kmailplugin.la
libkontact_kmailplugin_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkontact_kmailplugin_la_LIBADD = $(top_builddir)/kontact/interfaces/libkpinterfaces.la $(LIB_KPARTS)

libkontact_kmailplugin_la_SOURCES = kmail_plugin.cpp kmailIface.stub \
                                    summarywidget.cpp summarywidget.skel

kmailIface_DCOPIDLNG = true

summarywidget_DCOPIDLNG = true

METASOURCES = AUTO

servicedir = $(kde_servicesdir)/kontact
service_DATA = kmailplugin.desktop

kmailIface_DIR = $(top_srcdir)/kmail
