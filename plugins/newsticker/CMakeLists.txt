kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kontact/interfaces ${CMAKE_SOURCE_DIR}/libkdepim ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kontact_newstickerplugin_PART_SRCS newsticker_plugin.cpp summarywidget.cpp )

kde4_automoc(${kontact_newstickerplugin_PART_SRCS})

kde4_add_dcop_skels(kontact_newstickerplugin_PART_SRCS summarywidget.h )

kde4_add_plugin(kontact_newstickerplugin WITH_PREFIX ${kontact_newstickerplugin_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kontact_newstickerplugin )

target_link_libraries(kontact_newstickerplugin  ${QT_AND_KDECORE_LIBS} kparts kdeui kdepim kpinterfaces )

install_targets(${LIB_INSTALL_DIR}/kde4 kontact_newstickerplugin )


########### next target ###############

set(kcm_kontactknt_PART_SRCS kcmkontactknt.cpp )

kde4_automoc(${kcm_kontactknt_PART_SRCS})

kde4_add_plugin(kcm_kontactknt ${kcm_kontactknt_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_kontactknt )

target_link_libraries(kcm_kontactknt  ${QT_AND_KDECORE_LIBS} kdeui )

install_targets(${LIB_INSTALL_DIR}/kde4 kcm_kontactknt )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR}/kontact FILES newstickerplugin.desktop )
install_files( ${SERVICES_INSTALL_DIR} FILES kcmkontactknt.desktop )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/kontact/interfaces \
#     -I$(top_srcdir)/libkdepim \
#     -I$(top_srcdir) $(all_includes)
#
#kde_module_LTLIBRARIES = libkontact_newstickerplugin.la kcm_kontactknt.la
#libkontact_newstickerplugin_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkontact_newstickerplugin_la_LIBADD = $(LIB_KPARTS) $(LIB_KDEUI) \
#                                        $(top_builddir)/libkdepim/libkdepim.la ../../interfaces/libkpinterfaces.la
#
#libkontact_newstickerplugin_la_SOURCES = newsticker_plugin.cpp \
#                                         summarywidget.cpp summarywidget.skel
#
#kcm_kontactknt_la_SOURCES = kcmkontactknt.cpp
#kcm_kontactknt_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) \
#                            -avoid-version -no-undefined
#kcm_kontactknt_la_LIBADD = $(LIB_KDEUI)
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)/kontact
#service_DATA = newstickerplugin.desktop
#
#kde_services_DATA = kcmkontactknt.desktop
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/kcmkontactnt.pot
