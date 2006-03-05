kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kpinterfaces_LIB_SRCS 
   core.cpp 
   plugin.cpp 
   summary.cpp 
   uniqueapphandler.cpp )

kde4_automoc(${kpinterfaces_LIB_SRCS})

kde4_add_library(kpinterfaces SHARED ${kpinterfaces_LIB_SRCS})

target_link_libraries(kpinterfaces  ${QT_AND_KDECORE_LIBS} kdepim kparts )

set_target_properties(kpinterfaces PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install_targets(${LIB_INSTALL_DIR} kpinterfaces )


########### install files ###############

install_files( /include/kontact FILES core.h plugin.h summary.h )
install_files( ${SERVICETYPES_INSTALL_DIR} FILES kontactplugin.desktop )

kde4_footer()



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
