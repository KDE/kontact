SUBDIRS = interfaces plugins src pics

DOXYGEN_SET_RECURSIVE = YES
DOXYGEN_REFERENCES = kdeui kparts libkcal
include $(top_srcdir)/doc/api/Doxyfile.am


messages: rc.cpp
	$(EXTRACTRC) src/*.kcfg >> rc.cpp
	$(XGETTEXT) rc.cpp src/*.cpp interfaces/*.cpp plugins/*/*.cpp plugins/*/*.h -o $(podir)/kontact.pot
