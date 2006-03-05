kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kontact/interfaces ${CMAKE_SOURCE_DIR}/libkdepim ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kontact_specialdatesplugin_PART_SRCS specialdates_plugin.cpp sdsummarywidget.cpp )

kde4_automoc(${kontact_specialdatesplugin_PART_SRCS})

kde4_add_plugin(kontact_specialdatesplugin WITH_PREFIX ${kontact_specialdatesplugin_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kontact_specialdatesplugin )

target_link_libraries(kontact_specialdatesplugin  ${QT_AND_KDECORE_LIBS} kpinterfaces kdepim kholidays korganizer_calendar kaddressbook )

install_targets(${LIB_INSTALL_DIR}/kde4 kontact_specialdatesplugin )


########### next target ###############

set(kcm_sdsummary_PART_SRCS kcmsdsummary.cpp )

kde4_automoc(${kcm_sdsummary_PART_SRCS})

kde4_add_ui_files(kcm_sdsummary_PART_SRCS sdsummaryconfig_base.ui )

kde4_add_plugin(kcm_sdsummary ${kcm_sdsummary_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_sdsummary )

target_link_libraries(kcm_sdsummary  ${QT_AND_KDECORE_LIBS} kdeui )

install_targets(${LIB_INSTALL_DIR}/kde4 kcm_sdsummary )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR}/kontact FILES specialdatesplugin.desktop )
install_files( ${SERVICES_INSTALL_DIR} FILES kcmsdsummary.desktop )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/kontact/interfaces \
#  -I$(top_srcdir)/libkdepim \
#  -I$(top_srcdir) $(all_includes)
#
#kde_module_LTLIBRARIES = libkontact_specialdatesplugin.la kcm_sdsummary.la
#libkontact_specialdatesplugin_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkontact_specialdatesplugin_la_LIBADD = \
#        $(top_builddir)/kontact/interfaces/libkpinterfaces.la \
#        $(top_builddir)/libkdepim/libkdepim.la \
#        $(top_builddir)/libkholidays/libkholidays.la \
#        $(top_builddir)/korganizer/libkorganizer_calendar.la \
#        $(top_builddir)/kaddressbook/libkaddressbook.la
#
#libkontact_specialdatesplugin_la_SOURCES = specialdates_plugin.cpp \
#                                           sdsummarywidget.cpp
#
#kcm_sdsummary_la_SOURCES = kcmsdsummary.cpp sdsummaryconfig_base.ui
#kcm_sdsummary_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) \
#                            -avoid-version -no-undefined
#kcm_sdsummary_la_LIBADD = $(LIB_KDEUI)
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)/kontact
#service_DATA = specialdatesplugin.desktop
#
#kde_services_DATA = kcmsdsummary.desktop
#
#specialdatesiface_DIR = $(top_srcdir)/specialdates
#kmailIface_DIR = $(top_srcdir)/kmail
#kmailIface_DCOPIDLNG = true
