SUBDIRS = interfaces plugins src pics

include ../admin/Doxyfile.am


messages: rc.cpp
	$(EXTRACTRC) src/*.kcfg >> rc.cpp
	$(XGETTEXT) src/*.cpp interfaces/*.cpp plugins/*/*.cpp plugins/*/*.h -o $(podir)/kontact.pot
