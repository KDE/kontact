# top_srcdir/korganizer is a hack, to get korganizeriface.h
INCLUDES = -I$(top_srcdir)/kontact/interfaces \
  -I$(top_builddir)/libkdepim -I$(top_builddir)/libkdepim \
  -I$(top_srcdir)/korganizer \
  -I$(top_srcdir) $(all_includes)

kde_module_LTLIBRARIES = libkontact_korganizerplugin.la \
                         libkontact_todoplugin.la \
                         libkontact_journalplugin.la \
                         kcm_korgsummary.la

noinst_LTLIBRARIES = libcommon.la
libcommon_la_SOURCES = korg_uniqueapp.cpp

libkontact_korganizerplugin_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkontact_korganizerplugin_la_LIBADD = libcommon.la \
    $(top_builddir)/kontact/interfaces/libkpinterfaces.la $(LIB_KPARTS) \
    $(top_builddir)/korganizer/libkorganizer_calendar.la
libkontact_korganizerplugin_la_SOURCES = korganizerplugin.cpp \
                                         kcalendariface.stub \
                                         summarywidget.cpp

libkontact_todoplugin_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkontact_todoplugin_la_LIBADD = libcommon.la \
    $(top_builddir)/kontact/interfaces/libkpinterfaces.la $(LIB_KPARTS) \
    $(top_builddir)/korganizer/libkorganizer_calendar.la
libkontact_todoplugin_la_SOURCES = todoplugin.cpp kcalendariface.stub \
                                   todosummarywidget.cpp

libkontact_journalplugin_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkontact_journalplugin_la_LIBADD = libcommon.la \
    $(top_builddir)/kontact/interfaces/libkpinterfaces.la $(LIB_KPARTS) \
    $(top_builddir)/korganizer/libkorganizer_calendar.la
libkontact_journalplugin_la_SOURCES = journalplugin.cpp kcalendariface.stub

kcm_korgsummary_la_SOURCES = kcmkorgsummary.cpp
kcm_korgsummary_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) \
                             $(all_libraries) \
                             -avoid-version -no-undefined
kcm_korgsummary_la_LIBADD = $(LIB_KDEUI)

METASOURCES = AUTO

kcalendariface_DIR = $(top_srcdir)/korganizer

servicedir = $(kde_servicesdir)/kontact
service_DATA = korganizerplugin.desktop todoplugin.desktop journalplugin.desktop

kde_services_DATA = kcmkorgsummary.desktop

KDE_ICON = AUTO
