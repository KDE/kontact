INCLUDES = -I$(top_srcdir)/libkdepim -I$(top_srcdir)/kontact/interfaces \
  $(all_includes)

lib_LTLIBRARIES = libkontact.la
libkontact_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -version-info 1:0
libkontact_la_LIBADD = $(LIB_KDECORE)
libkontact_la_SOURCES = prefs.cpp

bin_PROGRAMS = kontact

kontact_METASOURCES = AUTO
kontact_LDFLAGS = $(all_libraries) $(KDE_RPATH) -lkutils
kontact_LDADD = $(LIB_KPARTS) $(top_builddir)/libkdepim/libkdepim.la \
  $(top_builddir)/kontact/interfaces/libkpinterfaces.la libkontact.la
kontact_SOURCES = main.cpp mainwindow.cpp splash.cpp sidepanebase.cpp \
  sidepane.cpp iconsidepane.cpp aboutdialog.cpp

kde_module_LTLIBRARIES = kcm_kontact.la

kcm_kontact_la_SOURCES = kcmkontact.cpp
kcm_kontact_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
kcm_kontact_la_LIBADD = libkontact.la $(top_builddir)/libkdepim/libkdepim.la \
  $(LIB_KDECORE)

rcdir = $(kde_datadir)/kontact
rc_DATA = kontactui.rc

xdg_apps_DATA = Kontact.desktop

hidden_apps_DATA =  kontactconfig.desktop
hidden_appsdir = $(kde_appsdir)/.hidden

KDE_ICON = AUTO
