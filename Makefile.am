SUBDIRS = interfaces plugins src pics

include $(top_srcdir)/doc/api/Doxyfile.am


messages: rc.cpp
	$(EXTRACTRC) src/*.kcfg >> rc.cpp
	$(XGETTEXT) rc.cpp src/*.cpp interfaces/*.cpp plugins/*/*.cpp plugins/*/*.h -o $(podir)/kontact.pot
