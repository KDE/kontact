INCLUDES = -I$(top_srcdir)/kontact/interfaces -I$(top_srcdir) $(all_includes)

kde_module_LTLIBRARIES = libkpknotesplugin.la
libkpknotesplugin_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkpknotesplugin_la_LIBADD = $(top_builddir)/kontact/interfaces/libkpinterfaces.la \
                              $(LIB_KPARTS) $(top_builddir)/libkdepim/libkdepim.la

libkpknotesplugin_la_SOURCES = knotes_plugin.cpp knotes_part.cpp \
                               summarywidget.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kpknotesplugin.desktop

rc_DATA = kpknotesplugin.rc
rcdir = $(kde_datadir)/kpknotesplugin

partdir = $(kde_datadir)/kontact
part_DATA = knotes_part.rc

