
include_directories( ${CMAKE_SOURCE_DIR}/kontact/interfaces ${CMAKE_SOURCE_DIR}/karm  )


########### next target ###############

set(kontact_karm_PART_SRCS karm_plugin.cpp )

kde4_automoc(${kontact_karm_PART_SRCS})

qt4_add_dbus_interfaces(kontact_karm_PART_SRCS ${CMAKE_SOURCE_DIR}/karm/org.kde.karm.Karm.xml )

kde4_add_plugin(kontact_karm WITH_PREFIX ${kontact_karm_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kontact_karm )

target_link_libraries(kontact_karm  ${KDE4_KDECORE_LIBS} kpinterfaces )

install(TARGETS kontact_karm  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES karmplugin.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kontact)




#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/kontact/interfaces -I$(top_srcdir)/karm -I$(top_srcdir) $(all_includes)
#
#kde_module_LTLIBRARIES = libkontact_karm.la
#libkontact_karm_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkontact_karm_la_LIBADD = $(top_builddir)/kontact/interfaces/libkpinterfaces.la
#
#libkontact_karm_la_SOURCES = karm_plugin.cpp karmdcopiface.stub
#
#METASOURCES = AUTO
#
#karmdcopiface_DIR = $(top_srcdir)/karm
#
#servicedir = $(kde_servicesdir)/kontact
#service_DATA = karmplugin.desktop
#
#DISTCLEANFILES = karmdcopiface.h karmdcopiface.stub
