kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kontact/interfaces ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kptestplugin_PART_SRCS test_plugin.cpp test_part.cpp )

kde4_automoc(${kptestplugin_PART_SRCS})

kde4_add_dcop_stubs(kptestplugin_PART_SRCS kaddressbookiface.h )

kde4_add_plugin(kptestplugin WITH_PREFIX ${kptestplugin_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kptestplugin )

target_link_libraries(kptestplugin  ${QT_AND_KDECORE_LIBS} kpinterfaces kparts )

install_targets(${LIB_INSTALL_DIR}/kde4 kptestplugin )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kptestplugin.desktop )
install_files( ${DATA_INSTALL_DIR}/kptestplugin FILES kptestplugin.rc )
install_files( ${DATA_INSTALL_DIR}/testpart FILES testpartui.rc )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/kontact/interfaces $(all_includes)
#
#kde_module_LTLIBRARIES = libkptestplugin.la
#libkptestplugin_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkptestplugin_la_LIBADD = $(top_builddir)/kontact/interfaces/libkpinterfaces.la $(LIB_KPARTS)
#
#libkptestplugin_la_SOURCES = test_plugin.cpp test_part.cpp kaddressbookiface.stub
#
#kaddressbookiface_DIR = $(top_srcdir)/kaddressbook
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = kptestplugin.desktop
#
#rc_DATA = kptestplugin.rc
#rcdir = $(kde_datadir)/kptestplugin
#
#kpartrc_DATA = testpartui.rc
#kpartrcdir = $(kde_datadir)/testpart
