INCLUDES = -I$(top_srcdir)/kontact/interfaces $(all_includes)

kde_module_LTLIBRARIES = libkontact_multisynk.la
libkontact_multisynk_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkontact_multisynk_la_LIBADD = $(top_builddir)/kontact/interfaces/libkpinterfaces.la

libkontact_multisynk_la_SOURCES = multisynk_plugin.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)/kontact
service_DATA = multisynk.desktop
