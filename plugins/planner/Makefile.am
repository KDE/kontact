INCLUDES = -I$(top_srcdir)/kontact/interfaces \
  -I$(top_srcdir)/libkdepim \
  -I$(top_srcdir) $(all_includes)

kde_module_LTLIBRARIES = libkontact_plannerplugin.la kcm_planner.la
libkontact_plannerplugin_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkontact_plannerplugin_la_LIBADD = \
        $(top_builddir)/kontact/interfaces/libkpinterfaces.la \
        $(top_builddir)/korganizer/libkorganizer_calendar.la

libkontact_plannerplugin_la_SOURCES = plannerplugin.cpp \
                                      planner.cpp \
                                      korganizeriface.stub

kcm_planner_la_SOURCES = kcmplanner.cpp
kcm_planner_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) \
                            -avoid-version -no-undefined
kcm_planner_la_LIBADD = $(LIB_KDEUI)

METASOURCES = AUTO

kcalendariface_DIR = $(top_srcdir)/korganizer

servicedir = $(kde_servicesdir)/kontact
service_DATA = plannerplugin.desktop

kde_services_DATA = kcmplanner.desktop

planneriface_DIR = $(top_srcdir)/planner
korganizeriface_DIR = $(top_srcdir)/korganizer
korganizeriface_DCOPIDLNG = true
kmailIface_DIR = $(top_srcdir)/kmail
kmailIface_DCOPIDLNG = true
