
include_directories(  ${CMAKE_SOURCE_DIR}/kontact/interfaces ${CMAKE_SOURCE_DIR}/kmobiletools/src ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############

set(kontact_kmobiletools_PART_SRCS kmobiletools_plugin.cpp )

kde4_automoc(${kontact_kmobiletools_PART_SRCS})

kde4_add_plugin(kontact_kmobiletools WITH_PREFIX ${kontact_kmobiletools_PART_SRCS})

target_link_libraries(kontact_kmobiletools  ${KDE4_KDECORE_LIBS} kmobiletools kpinterfaces )

install(TARGETS kontact_kmobiletools DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES  kmobiletools.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kontact )



#original Makefile.am contents follow:

#KDE_OPTIONS = nofinal
#
#INCLUDES = -I$(top_srcdir) $(all_includes) \
#           -I../kmobiletools/libkmobiletools
#
#kde_module_LTLIBRARIES = libkontact_kmobiletools.la
#libkontact_kmobiletools_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkontact_kmobiletools_la_LIBADD = ../kmobiletools/libkmobiletools/libkmobiletools.la \
#				-lkpinterfaces
#
#libkontact_kmobiletools_la_SOURCES = kmobiletools_plugin.cpp
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)/kontact
#service_DATA = kmobiletools.desktop
#KDE_CXXFLAGS = -UQT_NO_ASCII_CAST
#
