MAINTAINERCLEANFILES = Makefile.in

desktopdir = $(datadir)/applications
dist_desktop_DATA = omnewrotate.desktop

appicondir = $(datadir)/pixmaps
dist_appicon_DATA = omnewrotate.png

x11scriptdir = @XSESSIONDIR@
x11script_in_files = 89omnewrotate.in
x11script_DATA = $(x11script_in_files:.in=)

initscriptdir = ${sysconfdir}/init.d
initscript_in_files = omnewrotate.in
initscript_DATA = $(initscript_in_files:.in=)

suspendscriptdir = @APMDIR@/suspend.d
suspendscript_in_files = 11omnewrotate-suspend.in
suspendscript_DATA = $(suspendscript_in_files:.in=)

resumescriptdir = @APMDIR@/resume.d
resumescript_in_files = 89omnewrotate-resume.in
resumescript_DATA = $(resumescript_in_files:.in=)

omconfdir = @CONFIGDIR@
omconf_in_files = omnewrotate.conf.in
omconf_DATA = $(omconf_in_files:.in=)

toggledir = @BINDIR@
toggle_in_files = omnewrotate.sh.in
toggle_DATA = $(toggle_in_files:.in=)

install-data-hook: 89omnewrotate-resume
	/bin/chmod 0755 $(DESTDIR)/@XSESSIONDIR@/89omnewrotate
	/bin/chmod 0755 $(DESTDIR)@APMDIR@/resume.d/89omnewrotate-resume
	/bin/chmod 0755 $(DESTDIR)@APMDIR@/suspend.d/11omnewrotate-suspend
	/bin/chmod 0755 $(DESTDIR)/@BINDIR@/omnewrotate.sh
	/bin/chmod 0755 $(DESTDIR)/${sysconfdir}/init.d/omnewrotate
