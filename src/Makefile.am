INCLUDES = -I$(top_srcdir)/kaplan/interfaces $(all_includes)

bin_PROGRAMS = kaplan

kaplan_SOURCES = main.cpp core.cpp navigator.cpp
kaplan_METASOURCES = AUTO
kaplan_LDFLAGS = $(all_libraries) $(KDE_RPATH) 
kaplan_LDADD = $(LIB_KPARTS) $(top_builddir)/kaplan/interfaces/libkpinterfaces.la

rcdir = $(kde_datadir)/kaplan
rc_DATA = kaplanrc


