INCLUDES = -I$(top_srcdir)/kontact/interfaces -I$(top_srcdir)/knode $(all_includes)

kde_module_LTLIBRARIES = libkpknodeplugin.la
libkpknodeplugin_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkpknodeplugin_la_LIBADD = $(top_builddir)/kontact/interfaces/libkpinterfaces.la $(LIB_KPARTS)

libkpknodeplugin_la_SOURCES = knode_plugin.cpp 

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kpknodeplugin.desktop

knodeIface_DIR = $(top_srcdir)/knode

rc_DATA = kpknodeplugin.rc
rcdir = $(kde_datadir)/kpknodeplugin
