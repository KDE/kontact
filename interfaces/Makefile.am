INCLUDES = $(all_includes)

lib_LTLIBRARIES = libkpinterfaces.la

libkpinterfaces_la_SOURCES = core.cpp plugin.cpp
libkpinterfaces_la_LDFLAGS = $(all_libraries) -no-undefined -version-info 1:0:0
libkpinterfaces_la_LIBADD = $(LIB_KPARTS)

kpincludedir = $(includedir)/kontact
kpinclude_HEADERS = core.h plugin.h

METASOURCES = AUTO

servicetypedir = $(kde_servicetypesdir)
servicetype_DATA = kontactplugin.desktop
