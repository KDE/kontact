kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kontact/interfaces ${CMAKE_SOURCE_DIR}/libkleo ${CMAKE_SOURCE_DIR}/libkdepim ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kontact_summaryplugin_PART_SRCS summaryview_plugin.cpp summaryview_part.cpp dropwidget.cpp )

kde4_automoc(${kontact_summaryplugin_PART_SRCS})

kde4_add_plugin(kontact_summaryplugin WITH_PREFIX ${kontact_summaryplugin_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kontact_summaryplugin )

target_link_libraries(kontact_summaryplugin  ${QT_AND_KDECORE_LIBS} kutils kpinterfaces kparts kdepim kpimidentities )

install_targets(${LIB_INSTALL_DIR}/kde4 kontact_summaryplugin )


########### next target ###############

set(kcm_kontactsummary_PART_SRCS kcmkontactsummary.cpp )

kde4_automoc(${kcm_kontactsummary_PART_SRCS})

kde4_add_plugin(kcm_kontactsummary ${kcm_kontactsummary_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_kontactsummary )

target_link_libraries(kcm_kontactsummary  ${QT_AND_KDECORE_LIBS} kdeui kutils )

install_targets(${LIB_INSTALL_DIR}/kde4 kcm_kontactsummary )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR}/kontact FILES summaryplugin.desktop )
install_files( ${SERVICES_INSTALL_DIR} FILES kcmkontactsummary.desktop )
install_files( ${DATA_INSTALL_DIR}/kontactsummary FILES kontactsummary_part.rc )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/kontact/interfaces -I$(top_srcdir)/libkleo \
#  -I$(top_srcdir)/libkdepim \
#  -I$(top_srcdir) $(all_includes)
#
#kde_module_LTLIBRARIES = libkontact_summaryplugin.la kcm_kontactsummary.la
#libkontact_summaryplugin_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkontact_summaryplugin_la_LIBADD = -lkutils \
#  $(top_builddir)/kontact/interfaces/libkpinterfaces.la $(LIB_KPARTS) \
#  $(top_builddir)/libkdepim/libkdepim.la $(top_builddir)/libkpimidentities/libkpimidentities.la
#
#libkontact_summaryplugin_la_SOURCES = summaryview_plugin.cpp summaryview_part.cpp dropwidget.cpp
#
#kcm_kontactsummary_la_SOURCES = kcmkontactsummary.cpp
#kcm_kontactsummary_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) \
#                                -avoid-version -no-undefined
#kcm_kontactsummary_la_LIBADD = $(LIB_KDEUI) $(LIB_KUTILS)
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)/kontact
#service_DATA = summaryplugin.desktop
#
#kde_services_DATA = kcmkontactsummary.desktop
#
#kpartrcdir = $(kde_datadir)/kontactsummary
#kpartrc_DATA = kontactsummary_part.rc
