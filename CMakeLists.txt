kde4_header()

add_subdirectory( interfaces ) 
add_subdirectory( plugins ) 
add_subdirectory( src ) 
add_subdirectory( pics ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############


kde4_create_doxygen_docs( REFERENCES kdeui kparts libkcal  )

kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = interfaces plugins src pics
#
#DOXYGEN_REFERENCES = kdeui kparts libkcal
#include $(top_srcdir)/admin/Doxyfile.am
#
#
#messages: rc.cpp
#	$(EXTRACTRC) src/*.kcfg >> rc.cpp
#	$(XGETTEXT) rc.cpp src/*.cpp interfaces/*.cpp plugins/*/*.cpp plugins/*/*.h -o $(podir)/kontact.pot
