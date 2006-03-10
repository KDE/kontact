kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kontact/interfaces ${CMAKE_SOURCE_DIR}/kmail ${CMAKE_SOURCE_DIR}/libkdepim ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kontact_kmailplugin_PART_SRCS kmail_plugin.cpp summarywidget.cpp )

kde4_automoc(${kontact_kmailplugin_PART_SRCS})

kde4_add_dcop_skels(kontact_kmailplugin_PART_SRCS summarywidget.h )

kde4_add_dcop_stubs(kontact_kmailplugin_PART_SRCS ${CMAKE_SOURCE_DIR}/kmail/kmailIface.h )

kde4_add_plugin(kontact_kmailplugin WITH_PREFIX ${kontact_kmailplugin_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kontact_kmailplugin )

target_link_libraries(kontact_kmailplugin  ${QT_AND_KDECORE_LIBS} kpinterfaces kcal kparts )

install_targets(${LIB_INSTALL_DIR}/kde4 kontact_kmailplugin )


########### next target ###############

set(kcm_kmailsummary_PART_SRCS kcmkmailsummary.cpp )

kde4_automoc(${kcm_kmailsummary_PART_SRCS})

kde4_add_plugin(kcm_kmailsummary ${kcm_kmailsummary_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_kmailsummary )

target_link_libraries(kcm_kmailsummary  ${QT_AND_KDECORE_LIBS} kdeui )

install_targets(${LIB_INSTALL_DIR}/kde4 kcm_kmailsummary )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR}/kontact FILES kmailplugin.desktop )
install_files( ${SERVICES_INSTALL_DIR} FILES kcmkmailsummary.desktop )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/kontact/interfaces -I$(top_srcdir)/kmail \
#           -I$(top_srcdir)/libkdepim \
#           -I$(top_srcdir) $(all_includes)
#
#kde_module_LTLIBRARIES = libkontact_kmailplugin.la kcm_kmailsummary.la
#
#libkontact_kmailplugin_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkontact_kmailplugin_la_LIBADD = $(top_builddir)/kontact/interfaces/libkpinterfaces.la $(top_builddir)/libkcal/libkcal.la $(LIB_KPARTS)
#
#libkontact_kmailplugin_la_SOURCES = kmail_plugin.cpp kmailIface.stub \
#                                    summarywidget.cpp summarywidget.skel
#
#kcm_kmailsummary_la_SOURCES = kcmkmailsummary.cpp
#kcm_kmailsummary_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) \
#                              -avoid-version -no-undefined
#kcm_kmailsummary_la_LIBADD = $(LIB_KDEUI)
#
#kmailIface_DCOPIDLNG = true
#
#summarywidget_DCOPIDLNG = true
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)/kontact
#service_DATA = kmailplugin.desktop
#
#kde_services_DATA = kcmkmailsummary.desktop
#
#kmailIface_DIR = $(top_srcdir)/kmail
