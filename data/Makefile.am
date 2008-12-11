MAINTAINERCLEANFILES = Makefile.in

desktopdir = $(datadir)/applications
dist_desktop_DATA = omnewrotate.desktop

appicondir = $(datadir)/pixmaps
dist_appicon_DATA = omnewrotate.png

x11scriptdir = @XSESSIONDIR@
x11script_in_files = 89omnewrotate.in
x11script_DATA = $(script_in_files:.in=)
install-data-hook: 89omnewrotate
	/bin/chmod 0755 $(DESTDIR)/@XSESSIONDIR@/89omnewrotate

suspendscriptdir = /etc/apm/suspend.d
suspendscript_in_files = 11omnewrotate-suspend.in
suspendscript_DATA = $(suspendscript_in_files:.in=)
install-data-hook: 11omnewrotate-suspend
	/bin/chmod 0755 $(DESTDIR)/etc/apm/suspend.d/11omnewrotate-suspend

resumescriptdir = /etc/apm/resume.d
resumescript_in_files = 89omnewrotate-resume.in
resumescript_DATA = $(resumescript_in_files:.in=)
install-data-hook: 89omnewrotate-resume
	/bin/chmod 0755 $(DESTDIR)/etc/apm/resume.d/89omnewrotate-resume
