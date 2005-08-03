INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/kontact/interfaces -I$(top_srcdir)/kontact/plugins/karm -I$(top_srcdir)/karm -I$(top_srcdir)/libkcal -I$(top_srcdir)/kresources/remote $(all_includes)

kde_module_LTLIBRARIES = libkontact_karm.la
libkontact_karm_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkontact_karm_la_LIBADD = $(top_builddir)/kontact/interfaces/libkpinterfaces.la $(top_builddir)/karm/libkarm_shared.la

libkontact_karm_la_SOURCES = karm_plugin.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)/kontact
service_DATA = karmplugin.desktop

karmdcopiface_DIR = $(top_srcdir)/karm
karmdcopiface_DCOPIDLNG = true
