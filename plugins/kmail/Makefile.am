INCLUDES = -I$(top_srcdir)/kontact/interfaces -I$(top_srcdir)/kmail \
           -I$(top_builddir)/libkdepim -I$(top_builddir)/libkdepim \
           -I$(top_srcdir) $(all_includes)

kde_module_LTLIBRARIES = libkontact_kmailplugin.la kcm_kmailsummary.la

libkontact_kmailplugin_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkontact_kmailplugin_la_LIBADD = $(top_builddir)/kontact/interfaces/libkpinterfaces.la $(top_builddir)/libkcal/libkcal.la $(LIB_KPARTS)

libkontact_kmailplugin_la_SOURCES = kmail_plugin.cpp kmailIface.stub \
                                    summarywidget.cpp summarywidget.skel

kcm_kmailsummary_la_SOURCES = kcmkmailsummary.cpp
kcm_kmailsummary_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) \
                              -avoid-version -no-undefined
kcm_kmailsummary_la_LIBADD = $(LIB_KDEUI)

kmailIface_DCOPIDLNG = true

summarywidget_DCOPIDLNG = true

METASOURCES = AUTO

servicedir = $(kde_servicesdir)/kontact
service_DATA = kmailplugin.desktop

kde_services_DATA = kcmkmailsummary.desktop

kmailIface_DIR = $(top_srcdir)/kmail
