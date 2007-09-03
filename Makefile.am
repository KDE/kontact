SUBDIRS = interfaces plugins src pics profiles

DOXYGEN_REFERENCES = kdeui kparts libkcal
include $(top_srcdir)/admin/Doxyfile.am


messages: rc.cpp
	$(EXTRACTRC) src/*.kcfg >> rc.cpp
	$(XGETTEXT) rc.cpp src/*.cpp interfaces/*.cpp plugins/*/*.cpp plugins/*/*.h -o $(podir)/kontact.pot
