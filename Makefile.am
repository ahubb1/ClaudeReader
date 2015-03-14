SUBDIRS = src

#Custom (un)/install target
install: install-recursive
	install -m 0755 share/bdreader.desktop $(prefix)/share/applications
	install -m 0755 share/bdreader.xpm $(prefix)/share/pixmaps
	install -m 0755 share/bdreader32.xpm $(prefix)/share/pixmaps
	install -m 0755 doc/bdreader.1 $(prefix)/share/man/man1

uninstall: uninstall-recursive
	rm -f $(prefix)/bin/bdreader
	rm -f $(prefix)/share/applications/bdreader.desktop
	rm -f $(prefix)/share/pixmaps/bdreader.xpm
	rm -f $(prefix)/share/pixmaps/bdreader32.xpm
	rm -f $(prefix)/share/man/man1/bdreader.1
