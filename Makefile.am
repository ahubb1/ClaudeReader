SUBDIRS = src

#Custom (un)/install target
install: install-recursive
	install -m 0755 share/bdreader.desktop $(datarootdir)/applications
	install -m 0755 share/bdreader.xpm $(datarootdir)/pixmaps
	install -m 0755 share/bdreader32.xpm $(datarootdir)/pixmaps

uninstall: uninstall-recursive
	rm -f $(prefix)/bin/BDReader
	rm -f $(datarootdir)/applications/bdreader.desktop
	rm -f $(datarootdir)/pixmaps/bdreader.xpm
	rm -f $(datarootdir)/pixmaps/bdreader32.xpm
