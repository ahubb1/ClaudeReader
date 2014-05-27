SUBDIRS = src

#Custom (un)/install target
install: install-recursive
	test -s $(HOME)/.bdreader || { \
		install -d -m 777 $(HOME)/.bdreader; \
		install -m 0755 share/about $(HOME)/.bdreader; \
		install -m 0755 share/help $(HOME)/.bdreader; \
		touch $(HOME)/.bdreader/preferences; \
		chmod 777 $(HOME)/.bdreader/preferences; \
	}
	install -m 0755 share/bdreader.desktop $(datarootdir)/applications
	install -m 0755 share/bdreader.xpm $(datarootdir)/pixmaps
	install -m 0755 share/bdreader32.xpm $(datarootdir)/pixmaps

uninstall: uninstall-recursive
	rm -f $(prefix)/bin/BDReader
	rm -rf $(HOME)/.bdreader
	rm -f $(datarootdir)/applications/bdreader.desktop
	rm -f $(datarootdir)/pixmaps/bdreader.xpm
	rm -f $(datarootdir)/pixmaps/bdreader32.xpm
