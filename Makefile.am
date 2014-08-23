SUBDIRS = src

#Custom (un)/install target
install: install-recursive
	install -m 0755 share/bdreader.desktop $(datarootdir)/applications
	install -m 0755 share/bdreader.xpm $(datarootdir)/pixmaps
	install -m 0755 share/bdreader32.xpm $(datarootdir)/pixmaps
	install -m 0755 doc/bdreader.1 $(datarootdir)/man/man1

uninstall: uninstall-recursive
	rm -f $(prefix)/bin/bdreader
	rm -f $(datarootdir)/applications/bdreader.desktop
	rm -f $(datarootdir)/pixmaps/bdreader.xpm
	rm -f $(datarootdir)/pixmaps/bdreader32.xpm
	rm -f $(datarootdir)/man/man1/bdreader.1
