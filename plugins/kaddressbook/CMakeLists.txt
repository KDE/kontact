kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kontact/interfaces ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kontact_kaddressbookplugin_PART_SRCS kaddressbook_plugin.cpp )

kde4_automoc(${kontact_kaddressbookplugin_PART_SRCS})

kde4_add_dcop_stubs(kontact_kaddressbookplugin_PART_SRCS 
	${CMAKE_SOURCE_DIR}/kaddressbook/kaddressbookiface.h 
	${CMAKE_SOURCE_DIR}/kmail/kmailIface.h )

kde4_add_plugin(kontact_kaddressbookplugin WITH_PREFIX ${kontact_kaddressbookplugin_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kontact_kaddressbookplugin )

target_link_libraries(kontact_kaddressbookplugin  ${QT_AND_KDECORE_LIBS} kabc kpinterfaces kdepim )

install_targets(${LIB_INSTALL_DIR}/kde4 kontact_kaddressbookplugin )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR}/kontact FILES kaddressbookplugin.desktop )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/kontact/interfaces -I$(top_srcdir) $(all_includes)
#
#kde_module_LTLIBRARIES = libkontact_kaddressbookplugin.la
#libkontact_kaddressbookplugin_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -lkabc
#libkontact_kaddressbookplugin_la_LIBADD = $(top_builddir)/kontact/interfaces/libkpinterfaces.la $(top_builddir)/libkdepim/libkdepim.la
#
#libkontact_kaddressbookplugin_la_SOURCES = kaddressbook_plugin.cpp \
#                                           kaddressbookiface.stub \
#                                           kmailIface.stub
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)/kontact
#service_DATA = kaddressbookplugin.desktop
#
#kaddressbookiface_DIR = $(top_srcdir)/kaddressbook
#kmailIface_DIR = $(top_srcdir)/kmail
#kmailIface_DCOPIDLNG = true
