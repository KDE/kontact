INCLUDES = -I$(top_srcdir) $(all_includes)

lib_LTLIBRARIES = libkpinterfaces.la

libkpinterfaces_la_SOURCES = core.cpp plugin.cpp
libkpinterfaces_la_LDFLAGS = $(all_libraries) -no-undefined -version-info 1:0:0
libkpinterfaces_la_LIBADD = $(LIB_KPARTS) ../../libkdepim/libkdepim.la 

kpincludedir = $(includedir)/kontact
kpinclude_HEADERS = core.h plugin.h summary.h

METASOURCES = AUTO

servicetypedir = $(kde_servicetypesdir)
servicetype_DATA = kontactplugin.desktop
