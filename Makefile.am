SUBDIRS = interfaces plugins src

include ../admin/Doxyfile.am


messages:
	$(XGETTEXT) src/*.cpp interfaces/*.cpp plugins/*/*.cpp -o $(podir)/kontact.pot

