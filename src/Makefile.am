INTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = -DPKGDATADIR=\"$(pkgdatadir)\" \
                $(XRANDR_CFLAGS) \
                $(FRAMEWORKD_GLIB_CFLAGS) \
                -Wall

AM_LDFLAGS = @XRANDR_LIBS@ @FRAMEWORKD_GLIB_LIBS@

bin_PROGRAMS=omnewrotate

omnewrotate_SOURCES = \
	omnewrotate.c
