INCLUDES = -I$(top_srcdir)/kontact/interfaces \
  -I$(top_builddir)/libkdepim -I$(top_builddir)/libkdepim \
  -I$(top_srcdir) $(all_includes)

kde_module_LTLIBRARIES = libkontact_specialdatesplugin.la kcm_sdsummary.la
libkontact_specialdatesplugin_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkontact_specialdatesplugin_la_LIBADD = \
        $(top_builddir)/kontact/interfaces/libkpinterfaces.la \
        $(top_builddir)/libkdepim/libkdepim.la \
        $(top_builddir)/libkholidays/libkholidays.la \
        $(top_builddir)/korganizer/libkorganizer_calendar.la \
        $(top_builddir)/kaddressbook/libkaddressbook.la

libkontact_specialdatesplugin_la_SOURCES = specialdates_plugin.cpp \
                                           sdsummarywidget.cpp

kcm_sdsummary_la_SOURCES = kcmsdsummary.cpp
kcm_sdsummary_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) \
                            -avoid-version -no-undefined
kcm_sdsummary_la_LIBADD = $(LIB_KDEUI)

METASOURCES = AUTO

servicedir = $(kde_servicesdir)/kontact
service_DATA = specialdatesplugin.desktop

kde_services_DATA = kcmsdsummary.desktop

specialdatesiface_DIR = $(top_srcdir)/specialdates
kmailIface_DIR = $(top_srcdir)/kmail
kmailIface_DCOPIDLNG = true
