INCLUDES = -I$(top_srcdir)/kontact/interfaces -I$(top_builddir)/libkdepim \
   -I$(top_srcdir)/libkdepim -I$(top_srcdir) $(all_includes)

kde_module_LTLIBRARIES = libkontact_kaddressbookplugin.la kcm_kabsummary.la
libkontact_kaddressbookplugin_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -lkabc
libkontact_kaddressbookplugin_la_LIBADD = $(top_builddir)/kontact/interfaces/libkpinterfaces.la $(top_builddir)/libkdepim/libkdepim.la

libkontact_kaddressbookplugin_la_SOURCES = kaddressbook_plugin.cpp \
                                           kaddressbookiface.stub \
                                           kmailIface.stub \
                                           kabsummarywidget.cpp

kcm_kabsummary_la_SOURCES = kcmkabsummary.cpp
kcm_kabsummary_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) \
                            -avoid-version -no-undefined
kcm_kabsummary_la_LIBADD = $(LIB_KDEUI)

METASOURCES = AUTO

servicedir = $(kde_servicesdir)/kontact
service_DATA = kaddressbookplugin.desktop

kde_services_DATA = kcmkabsummary.desktop

kaddressbookiface_DIR = $(top_srcdir)/kaddressbook
kmailIface_DIR = $(top_srcdir)/kmail
kmailIface_DCOPIDLNG = true
