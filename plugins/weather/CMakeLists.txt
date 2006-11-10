
include_directories( ${CMAKE_SOURCE_DIR}/kontact/interfaces ${CMAKE_SOURCE_DIR}/libkdepim  )


########### next target ###############

set(kontact_weatherplugin_PART_SRCS weather_plugin.cpp summarywidget.cpp )

kde4_automoc(${kontact_weatherplugin_PART_SRCS})

#kde4_add_dcop_skels(kontact_weatherplugin_PART_SRCS summarywidget.h )

kde4_add_plugin(kontact_weatherplugin WITH_PREFIX ${kontact_weatherplugin_PART_SRCS})



target_link_libraries(kontact_weatherplugin ${KDE4_KDECORE_LIBS} ${KDE4_KPARTS_LIBS} kpinterfaces kdepim)

install(TARGETS kontact_weatherplugin  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES weatherplugin.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kontact)




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
