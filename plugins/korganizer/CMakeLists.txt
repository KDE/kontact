kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kontact/interfaces ${CMAKE_SOURCE_DIR}/libkdepim ${CMAKE_SOURCE_DIR}/korganizer ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kontact_korganizerplugin_PART_SRCS korganizerplugin.cpp apptsummarywidget.cpp )

kde4_automoc(${kontact_korganizerplugin_PART_SRCS})

kde4_add_dcop_stubs(kontact_korganizerplugin_PART_SRCS 
	${CMAKE_SOURCE_DIR}/korganizer/kcalendariface.h 
	${CMAKE_SOURCE_DIR}/korganizer/korganizeriface.h 
	)

kde4_add_plugin(kontact_korganizerplugin WITH_PREFIX ${kontact_korganizerplugin_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kontact_korganizerplugin )

target_link_libraries(kontact_korganizerplugin  ${QT_AND_KDECORE_LIBS} common kpinterfaces kparts korganizer_calendar )

install_targets(${LIB_INSTALL_DIR}/kde4 kontact_korganizerplugin )


########### next target ###############

set(kontact_todoplugin_PART_SRCS todoplugin.cpp todosummarywidget.cpp )

kde4_automoc(${kontact_todoplugin_PART_SRCS})

kde4_add_dcop_stubs(kontact_todoplugin_PART_SRCS kcalendariface.h korganizeriface.h )

kde4_add_plugin(kontact_todoplugin WITH_PREFIX ${kontact_todoplugin_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kontact_todoplugin )

target_link_libraries(kontact_todoplugin  ${QT_AND_KDECORE_LIBS} common kpinterfaces kparts korganizer_calendar )

install_targets(${LIB_INSTALL_DIR}/kde4 kontact_todoplugin )


########### next target ###############

set(kontact_journalplugin_PART_SRCS journalplugin.cpp )

kde4_automoc(${kontact_journalplugin_PART_SRCS})

kde4_add_dcop_stubs(kontact_journalplugin_PART_SRCS kcalendariface.h )

kde4_add_plugin(kontact_journalplugin WITH_PREFIX ${kontact_journalplugin_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kontact_journalplugin )

target_link_libraries(kontact_journalplugin  ${QT_AND_KDECORE_LIBS} common kpinterfaces kparts korganizer_calendar )

install_targets(${LIB_INSTALL_DIR}/kde4 kontact_journalplugin )


########### next target ###############

set(kcm_apptsummary_PART_SRCS kcmapptsummary.cpp )

kde4_automoc(${kcm_apptsummary_PART_SRCS})

kde4_add_ui3_files(kcm_apptsummary_PART_SRCS apptsummaryconfig_base.ui )

kde4_add_plugin(kcm_apptsummary ${kcm_apptsummary_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_apptsummary )

target_link_libraries(kcm_apptsummary  ${QT_AND_KDECORE_LIBS} kdeui )

install_targets(${LIB_INSTALL_DIR}/kde4 kcm_apptsummary )


########### next target ###############

set(kcm_todosummary_PART_SRCS kcmtodosummary.cpp )

kde4_automoc(${kcm_todosummary_PART_SRCS})

kde4_add_ui3_files(kcm_todosummary_PART_SRCS todosummaryconfig_base.ui )

kde4_add_plugin(kcm_todosummary ${kcm_todosummary_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_todosummary )

target_link_libraries(kcm_todosummary  ${QT_AND_KDECORE_LIBS} kdeui )

install_targets(${LIB_INSTALL_DIR}/kde4 kcm_todosummary )


########### next target ###############


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR}/kontact FILES korganizerplugin.desktop todoplugin.desktop journalplugin.desktop )
install_files( ${SERVICES_INSTALL_DIR} FILES kcmapptsummary.desktop kcmtodosummary.desktop )

kde4_install_icons( ${ICON_INSTALL_DIR}  hicolor )

kde4_footer()



#original Makefile.am contents follow:

## top_srcdir/korganizer is a hack, to get korganizeriface.h
#INCLUDES = -I$(top_srcdir)/kontact/interfaces \
#  -I$(top_srcdir)/libkdepim \
#  -I$(top_srcdir)/korganizer \
#  -I$(top_srcdir) $(all_includes)
#
#kde_module_LTLIBRARIES = libkontact_korganizerplugin.la \
#                         libkontact_todoplugin.la \
#                         libkontact_journalplugin.la \
#                         kcm_apptsummary.la kcm_todosummary.la
#
#noinst_LTLIBRARIES = libcommon.la
#libcommon_la_SOURCES = korg_uniqueapp.cpp
#
#libkontact_korganizerplugin_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkontact_korganizerplugin_la_LIBADD = libcommon.la \
#    $(top_builddir)/kontact/interfaces/libkpinterfaces.la $(LIB_KPARTS) \
#    $(top_builddir)/korganizer/libkorganizer_calendar.la
#libkontact_korganizerplugin_la_SOURCES = korganizerplugin.cpp \
#                                         kcalendariface.stub \
#                                         apptsummarywidget.cpp \
#                                         korganizeriface.stub
#
#korganizeriface_DIR = $(top_srcdir)/korganizer
#korganizeriface_DCOPIDLNG = true
#
#libkontact_todoplugin_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkontact_todoplugin_la_LIBADD = libcommon.la \
#    $(top_builddir)/kontact/interfaces/libkpinterfaces.la $(LIB_KPARTS) \
#    $(top_builddir)/korganizer/libkorganizer_calendar.la
#libkontact_todoplugin_la_SOURCES = todoplugin.cpp \
#                                   kcalendariface.stub \
#                                   todosummarywidget.cpp \
#                                   korganizeriface.stub
#
#libkontact_journalplugin_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkontact_journalplugin_la_LIBADD = libcommon.la \
#    $(top_builddir)/kontact/interfaces/libkpinterfaces.la $(LIB_KPARTS) \
#    $(top_builddir)/korganizer/libkorganizer_calendar.la
#libkontact_journalplugin_la_SOURCES = journalplugin.cpp kcalendariface.stub
#
#kcm_apptsummary_la_SOURCES = kcmapptsummary.cpp apptsummaryconfig_base.ui
#kcm_apptsummary_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) \
#                             $(all_libraries) \
#                             -avoid-version -no-undefined
#kcm_apptsummary_la_LIBADD = $(LIB_KDEUI)
#
#kcm_todosummary_la_SOURCES = kcmtodosummary.cpp todosummaryconfig_base.ui
#kcm_todosummary_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) \
#                             $(all_libraries) \
#                             -avoid-version -no-undefined
#kcm_todosummary_la_LIBADD = $(LIB_KDEUI)
#
#METASOURCES = AUTO
#
#kcalendariface_DIR = $(top_srcdir)/korganizer
#
#servicedir = $(kde_servicesdir)/kontact
#service_DATA = korganizerplugin.desktop todoplugin.desktop journalplugin.desktop
#
#kde_services_DATA = kcmapptsummary.desktop kcmtodosummary.desktop
#
#KDE_ICON = AUTO
