INCLUDES = -I$(top_srcdir)/kontact/interfaces -I$(top_srcdir)/akregator/src $(all_includes)

kde_module_LTLIBRARIES = libkontact_akregator.la
libkontact_akregator_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkontact_akregator_la_LIBADD = -lkpinterfaces $(LIB_KPARTS)

libkontact_akregator_la_SOURCES = akregator_plugin.cpp akregator_partiface.stub

METASOURCES = AUTO

akregator_partiface_DIR = $(top_srcdir)/akregator/src

servicedir = $(kde_servicesdir)/kontact
service_DATA = akregatorplugin.desktop akregatorplugin3.2.desktop

