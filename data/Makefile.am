MAINTAINERCLEANFILES = Makefile.in

desktopdir = $(datadir)/applications
dist_desktop_DATA = omnewrotate.desktop

appicondir = $(datadir)/pixmaps
dist_appicon_DATA = omnewrotate.png

initscriptdir = ${sysconfdir}/init.d
initscript_in_files = omnewrotate.in
initscript_DATA = $(initscript_in_files:.in=)

toggledir = @BINDIR@
toggle_in_files = omnewrotate.sh.in
toggle_DATA = $(toggle_in_files:.in=)

install-data-hook: omnewrotate.sh
	/bin/chmod 0755 $(DESTDIR)/@BINDIR@/omnewrotate.sh
	/bin/chmod 0755 $(DESTDIR)/${sysconfdir}/init.d/omnewrotate
