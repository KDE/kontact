INCLUDES = -I$(top_srcdir)/kontact/interfaces \
     -I$(top_srcdir)/libkdepim \
     -I$(top_srcdir) $(all_includes)

kde_module_LTLIBRARIES = libkontact_newstickerplugin.la kcm_kontactknt.la
libkontact_newstickerplugin_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkontact_newstickerplugin_la_LIBADD = $(LIB_KPARTS) $(LIB_KDEUI) \
                                        $(top_builddir)/libkdepim/libkdepim.la ../../interfaces/libkpinterfaces.la

libkontact_newstickerplugin_la_SOURCES = newsticker_plugin.cpp \
                                         summarywidget.cpp summarywidget.skel

kcm_kontactknt_la_SOURCES = kcmkontactknt.cpp
kcm_kontactknt_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) \
                            -avoid-version -no-undefined
kcm_kontactknt_la_LIBADD = $(LIB_KDEUI)

METASOURCES = AUTO

servicedir = $(kde_servicesdir)/kontact
service_DATA = newstickerplugin.desktop

kde_services_DATA = kcmkontactknt.desktop

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kcmkontactnt.pot
