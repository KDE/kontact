INCLUDES = -I$(top_srcdir)/kaplan/interfaces $(all_includes)

bin_PROGRAMS = kontact

kontact_SOURCES = main.cpp core.cpp navigator.cpp
kontact_METASOURCES = AUTO
kontact_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kontact_LDADD = $(LIB_KPARTS) $(top_builddir)/kaplan/interfaces/libkpinterfaces.la

rcdir = $(kde_datadir)/kontact
rc_DATA = kontactui.rc

kdelnkdir = $(kde_appsdir)/Applications
kdelnk_DATA = Kontact.desktop

KDE_ICON = AUTO
