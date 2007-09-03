INCLUDES = -I$(top_srcdir)/kontact/interfaces -I$(top_srcdir) $(all_includes)

kde_module_LTLIBRARIES = libkontact_kaddressbookplugin.la
libkontact_kaddressbookplugin_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -lkabc
libkontact_kaddressbookplugin_la_LIBADD = $(top_builddir)/kontact/interfaces/libkpinterfaces.la $(top_builddir)/libkdepim/libkdepim.la

libkontact_kaddressbookplugin_la_SOURCES = kaddressbook_plugin.cpp \
                                           kaddressbookiface.stub \
                                           kmailIface.stub

METASOURCES = AUTO

servicedir = $(kde_servicesdir)/kontact
service_DATA = kaddressbookplugin.desktop

kaddressbookiface_DIR = $(top_srcdir)/kaddressbook
kmailIface_DIR = $(top_srcdir)/kmail
kmailIface_DCOPIDLNG = true
