



########### next target ###############

set(kpinterfaces_LIB_SRCS 
   core.cpp 
   plugin.cpp 
   summary.cpp 
   uniqueapphandler.cpp )

kde4_automoc(${kpinterfaces_LIB_SRCS})

kde4_add_library(kpinterfaces SHARED ${kpinterfaces_LIB_SRCS})

target_link_libraries(kpinterfaces  ${KDE4_KDECORE_LIBS} kdepim kparts )

set_target_properties(kpinterfaces PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install(TARGETS kpinterfaces  DESTINATION ${LIB_INSTALL_DIR})


########### install files ###############

install( FILES core.h plugin.h summary.h  DESTINATION ${INCLUDE_INSTALL_DIR}/kontact)
install( FILES kontactplugin.desktop  DESTINATION ${SERVICETYPES_INSTALL_DIR})




#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir) $(all_includes)
#
#lib_LTLIBRARIES = libkpinterfaces.la
#
#libkpinterfaces_la_SOURCES = core.cpp plugin.cpp summary.cpp uniqueapphandler.cpp
#libkpinterfaces_la_LDFLAGS = $(all_libraries) -no-undefined -version-info 1:0:0
#libkpinterfaces_la_LIBADD = ../../libkdepim/libkdepim.la $(LIB_KPARTS)
#
#kpincludedir = $(includedir)/kontact
#kpinclude_HEADERS = core.h plugin.h summary.h
#
#METASOURCES = AUTO
#
#servicetypedir = $(kde_servicetypesdir)
#servicetype_DATA = kontactplugin.desktop
