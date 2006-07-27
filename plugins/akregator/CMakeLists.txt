
include_directories( ${CMAKE_SOURCE_DIR}/kontact/interfaces ${CMAKE_SOURCE_DIR}/akregator/src  )


########### next target ###############

set(kontact_akregator_PART_SRCS akregator_plugin.cpp )

kde4_automoc(${kontact_akregator_PART_SRCS})

#kde4_add_dcop_stubs(kontact_akregator_PART_SRCS ${CMAKE_SOURCE_DIR}/akregator/src/akregator_partiface.h )

kde4_add_plugin(kontact_akregator WITH_PREFIX ${kontact_akregator_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kontact_akregator )

target_link_libraries(kontact_akregator ${KDE4_KDECORE_LIBS} ${KDE4_KPARTS_LIBS} kpinterfaces)

install(TARGETS kontact_akregator  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES akregatorplugin.desktop akregatorplugin3.2.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kontact)




#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/kontact/interfaces -I$(top_srcdir)/akregator/src -I$(top_srcdir) $(all_includes)
#
#kde_module_LTLIBRARIES = libkontact_akregator.la
#libkontact_akregator_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkontact_akregator_la_LIBADD = $(top_builddir)/kontact/interfaces/libkpinterfaces.la $(LIB_KPARTS)
#libkontact_akregator_la_SOURCES = akregator_plugin.cpp akregator_partiface.stub
#
#METASOURCES = AUTO
#
#akregator_partiface_DIR = $(top_srcdir)/akregator/src
#
#servicedir = $(kde_servicesdir)/kontact
#service_DATA = akregatorplugin.desktop akregatorplugin3.2.desktop
#
