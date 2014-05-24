SUBDIRS = src

#Custom install target
install: install-recursive
	mkdir ~/.bdreader
	chmod 777 ~/.bdreader
	install -m 0755 about ~/.bdreader
	install -m 0755 help ~/.bdreader
	touch ~/.bdreader/preferences
	chmod 777 ~/.bdreader/preferences
	
uninstall: uninstall-recursive
	rm -f $(PREFIX)/bin/BDReader
	rm -rf ~/.bdreader
