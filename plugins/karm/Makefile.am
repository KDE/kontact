INCLUDES = -I$(top_srcdir)/kontact/interfaces -I$(top_srcdir)/karm -I$(top_srcdir) $(all_includes)

kde_module_LTLIBRARIES = libkontact_karm.la
libkontact_karm_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkontact_karm_la_LIBADD = $(top_builddir)/kontact/interfaces/libkpinterfaces.la

libkontact_karm_la_SOURCES = karm_plugin.cpp karmdcopiface.stub

METASOURCES = AUTO

karmdcopiface_DIR = $(top_srcdir)/karm

servicedir = $(kde_servicesdir)/kontact
service_DATA = karmplugin.desktop

DISTCLEANFILES = karmdcopiface.h karmdcopiface.stub
