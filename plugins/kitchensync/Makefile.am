INCLUDES = -I$(top_srcdir)/kontact/interfaces $(all_includes)

kde_module_LTLIBRARIES = libkontact_kitchensync.la
libkontact_kitchensync_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkontact_kitchensync_la_LIBADD = $(top_builddir)/kontact/interfaces/libkpinterfaces.la

libkontact_kitchensync_la_SOURCES = kitchensync_plugin.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)/kontact
service_DATA = kitchensync.desktop

kitchensynciface_DIR = $(top_srcdir)/kitchensync
