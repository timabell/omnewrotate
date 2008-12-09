NTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = -DPKGDATADIR=\"$(pkgdatadir)\" \
		-I ../ \
                @PTHREAD_CFLAGS@ \
                @XRANDR_CFLAGS@ \
                @FRAMEWORKD_GLIB_CFLAGS@ \
                -Wall

AM_LDFLAGS =	@PTHREAD_LIBS@ \
		@XRANDR_LIBS@ \
		@FRAMEWORKD_GLIB_LIBS@

bin_PROGRAMS = omnewrotate

omnewrotate_SOURCES = \
	omnewrotate.c
