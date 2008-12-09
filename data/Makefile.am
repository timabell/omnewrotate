MAINTAINERCLEANFILES = Makefile.in

desktopdir = $(datadir)/applications
dist_desktop_DATA = omnewrotate.desktop

appicondir = $(datadir)/pixmaps
dist_appicon_DATA = omnewrotate.png

scriptdir = @XSESSIONDIR@
script_in_files = 89omnewrotate.in
script_DATA = $(script_in_files:.in=)

install-data-hook: 89omnewrotate
	/bin/chmod 0755 $(DESTDIR)/@XSESSIONDIR@/89omnewrotate
