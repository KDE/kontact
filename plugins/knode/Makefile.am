INCLUDES = -I$(top_srcdir)/kontact/interfaces -I$(top_srcdir)/knode $(all_includes)

kde_module_LTLIBRARIES = libkontact_knodeplugin.la
libkontact_knodeplugin_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkontact_knodeplugin_la_LIBADD = $(top_builddir)/kontact/interfaces/libkpinterfaces.la $(LIB_KPARTS)

libkontact_knodeplugin_la_SOURCES = knode_plugin.cpp knodeiface.stub 

METASOURCES = AUTO

servicedir = $(kde_servicesdir)/kontact
service_DATA = knodeplugin.desktop

knodeiface_DIR = $(top_srcdir)/knode
