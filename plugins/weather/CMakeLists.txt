kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kontact/interfaces ${CMAKE_SOURCE_DIR}/libkdepim ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kontact_weatherplugin_PART_SRCS weather_plugin.cpp summarywidget.cpp )

kde4_automoc(${kontact_weatherplugin_PART_SRCS})

kde4_add_dcop_skels(kontact_weatherplugin_PART_SRCS summarywidget.h )

kde4_add_plugin(kontact_weatherplugin WITH_PREFIX ${kontact_weatherplugin_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kontact_weatherplugin )

target_link_libraries(kontact_weatherplugin  ${QT_AND_KDECORE_LIBS} kpinterfaces kparts kdepim )

install_targets(${LIB_INSTALL_DIR}/kde4 kontact_weatherplugin )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR}/kontact FILES weatherplugin.desktop )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/kontact/interfaces -I$(top_srcdir)/libkdepim -I$(top_srcdir) $(all_includes)
#
#kde_module_LTLIBRARIES = libkontact_weatherplugin.la
#libkontact_weatherplugin_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkontact_weatherplugin_la_LIBADD = $(top_builddir)/kontact/interfaces/libkpinterfaces.la $(LIB_KPARTS) $(top_builddir)/libkdepim/libkdepim.la
#
#libkontact_weatherplugin_la_SOURCES = weather_plugin.cpp summarywidget.cpp \
#                                      summarywidget.skel
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)/kontact
#service_DATA = weatherplugin.desktop
