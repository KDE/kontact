
add_subdirectory( interfaces ) 
add_subdirectory( plugins ) 
add_subdirectory( src ) 
add_subdirectory( pics ) 







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
