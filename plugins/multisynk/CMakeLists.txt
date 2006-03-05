kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kontact/interfaces ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kontact_multisynk_PART_SRCS multisynk_plugin.cpp )

kde4_automoc(${kontact_multisynk_PART_SRCS})

kde4_add_plugin(kontact_multisynk WITH_PREFIX ${kontact_multisynk_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kontact_multisynk )

target_link_libraries(kontact_multisynk  ${QT_AND_KDECORE_LIBS} kpinterfaces )

install_targets(${LIB_INSTALL_DIR}/kde4 kontact_multisynk )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR}/kontact FILES multisynk.desktop )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/kontact/interfaces $(all_includes)
#
#kde_module_LTLIBRARIES = libkontact_multisynk.la
#libkontact_multisynk_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkontact_multisynk_la_LIBADD = $(top_builddir)/kontact/interfaces/libkpinterfaces.la
#
#libkontact_multisynk_la_SOURCES = multisynk_plugin.cpp
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)/kontact
#service_DATA = multisynk.desktop
