SUBDIRS = interfaces plugins src pics data

include ../admin/Doxyfile.am


messages: rc.cpp
	$(EXTRACTRC) src/*.kcfg >> rc.cpp
	$(XGETTEXT) src/*.cpp interfaces/*.cpp plugins/*/*.cpp -o $(podir)/kontact.pot
