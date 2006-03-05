kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kontact/interfaces ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kontact_knotesplugin_PART_SRCS 
   knotes_plugin.cpp 
   knotes_part.cpp 
   summarywidget.cpp 
   knotetip.cpp )

kde4_automoc(${kontact_knotesplugin_PART_SRCS})

kde4_add_dcop_skels(kontact_knotesplugin_PART_SRCS KNotesIface.h )

kde4_add_plugin(kontact_knotesplugin WITH_PREFIX ${kontact_knotesplugin_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kontact_knotesplugin )

target_link_libraries(kontact_knotesplugin  ${QT_AND_KDECORE_LIBS} kpinterfaces kparts kdepim kcal kresources knotesresources knoteseditor )

install_targets(${LIB_INSTALL_DIR}/kde4 kontact_knotesplugin )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/knotes FILES knotes_part.rc )
install_files( ${SERVICES_INSTALL_DIR}/kontact FILES knotesplugin.desktop )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/kontact/interfaces -I$(top_srcdir) $(all_includes)
#
#kde_module_LTLIBRARIES = libkontact_knotesplugin.la
#libkontact_knotesplugin_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkontact_knotesplugin_la_LIBADD = $(top_builddir)/kontact/interfaces/libkpinterfaces.la \
#                                    $(LIB_KPARTS) $(top_builddir)/libkdepim/libkdepim.la \
#                                    $(top_builddir)/libkcal/libkcal.la -lkresources \
#                                    $(top_builddir)/knotes/libknotesresources.la \
#                                    $(top_builddir)/knotes/libknoteseditor.la
#
#libkontact_knotesplugin_la_SOURCES = knotes_plugin.cpp knotes_part.cpp summarywidget.cpp \
#                                     knotetip.cpp KNotesIface.skel
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)/kontact
#service_DATA = knotesplugin.desktop
#
#partdir = $(kde_datadir)/knotes
#part_DATA = knotes_part.rc
#
#KNotesIface_DIR = $(top_srcdir)/knotes
