INCLUDES = -I$(top_srcdir)/kontact/interfaces -I$(top_builddir)/libkdepim -I$(top_srcdir) $(all_includes)

kde_module_LTLIBRARIES = libkontact_knotesplugin.la
libkontact_knotesplugin_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkontact_knotesplugin_la_LIBADD = $(top_builddir)/kontact/interfaces/libkpinterfaces.la \
                                    $(LIB_KPARTS) $(top_builddir)/libkdepim/libkdepim.la \
                                    $(top_builddir)/libkcal/libkcal.la -lkresources \
                                    $(top_builddir)/knotes/libknotesresources.la \
                                    $(top_builddir)/knotes/libknoteseditor.la

libkontact_knotesplugin_la_SOURCES = knotes_plugin.cpp knotes_part.cpp summarywidget.cpp \
                                     knotetip.cpp KNotesIface.skel

METASOURCES = AUTO

servicedir = $(kde_servicesdir)/kontact
service_DATA = knotesplugin.desktop

partdir = $(kde_datadir)/knotes
part_DATA = knotes_part.rc

KNotesIface_DIR = $(top_srcdir)/knotes
