
add_subdirectory( about ) 

include_directories( ${CMAKE_SOURCE_DIR}/kontact/interfaces  )


########### next target ###############

set(kontact_LIB_SRCS 
   mainwindow.cpp 
   sidepanebase.cpp 
   iconsidepane.cpp 
   aboutdialog.cpp )

kde4_automoc(${kontact_LIB_SRCS})

kde4_add_kcfg_files(kontact_LIB_SRCS prefs.kcfgc )

kde4_add_library(kontact SHARED ${kontact_LIB_SRCS})

target_link_libraries(kontact  ${KDE4_KDECORE_LIBS} kdepim kpinterfaces kontact kparts kutils khtml )

set_target_properties(kontact PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install(TARGETS kontact  DESTINATION ${LIB_INSTALL_DIR})


########### next target ###############

set(kontact_bin_SRCS main.cpp )

kde4_automoc(${kontact_bin_SRCS})

kde4_add_executable(kontact_bin ${kontact_bin_SRCS})

set_target_properties(kontact_bin  PROPERTIES OUTPUT_NAME kontact)

target_link_libraries(kontact_bin  ${KDE4_KDECORE_LIBS} khtml ${KDE4_KDE3SUPPORT_LIBS} kontact)

install(TARGETS kontact_bin  DESTINATION ${BIN_INSTALL_DIR} )


########### next target ###############

set(kcm_kontact_PART_SRCS kcmkontact.cpp )

kde4_automoc(${kcm_kontact_PART_SRCS})

kde4_add_plugin(kcm_kontact ${kcm_kontact_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_kontact )

target_link_libraries(kcm_kontact  ${KDE4_KDECORE_LIBS} kontact kdepim )

install(TARGETS kcm_kontact  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES Kontact.desktop  DESTINATION ${XDG_APPS_DIR})
install( FILES kontact.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install( FILES kontactconfig.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES kontactui.rc  DESTINATION ${DATA_INSTALL_DIR}/kontact)
install( FILES kontact.setdlg  DESTINATION ${DATA_INSTALL_DIR}/kontact)

kde4_install_icons( ${ICON_INSTALL_DIR}  hicolor )




#original Makefile.am contents follow:

#SUBDIRS = about
#
#INCLUDES = -I$(top_srcdir)/kontact/interfaces -I$(top_srcdir) $(all_includes)
#
#lib_LTLIBRARIES = libkontact.la
#libkontact_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -version-info 1:0
#libkontact_la_LIBADD = $(LIB_KDECORE)
#libkontact_la_SOURCES = prefs.kcfgc
#
#bin_PROGRAMS = kontact
#
#kontact_METASOURCES = AUTO
#kontact_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#kontact_LDADD = $(top_builddir)/libkdepim/libkdepim.la \
#  $(top_builddir)/kontact/interfaces/libkpinterfaces.la libkontact.la \
#  $(LIB_KPARTS) $(LIB_KUTILS) $(LIB_KHTML)
#kontact_SOURCES = main.cpp mainwindow.cpp sidepanebase.cpp \
#  iconsidepane.cpp aboutdialog.cpp
#kontact_COMPILE_FIRST = prefs.h
#
#kde_module_LTLIBRARIES = kcm_kontact.la
#
#kcm_kontact_la_SOURCES = kcmkontact.cpp
#kcm_kontact_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
#kcm_kontact_la_LIBADD = libkontact.la $(top_builddir)/libkdepim/libkdepim.la
#kcm_kontact_la_COMPILE_FIRST = prefs.h
#
#
#rcdir = $(kde_datadir)/kontact
#rc_DATA = kontactui.rc
#
#xdg_apps_DATA = Kontact.desktop
#
#kde_kcfg_DATA = kontact.kcfg
#
#kde_services_DATA = kontactconfig.desktop
#
#kontactsetdlgdir = $(kde_datadir)/kontact
#kontactsetdlg_DATA = kontact.setdlg
#
#KDE_ICON = AUTO
