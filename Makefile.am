SUBDIRS = interfaces plugins src pics

include ../admin/Doxyfile.am


messages: rc.cpp
	$(XGETTEXT) src/*.cpp interfaces/*.cpp plugins/*/*.cpp -o $(podir)/kontact.pot
