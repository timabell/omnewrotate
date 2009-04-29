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

toggledir = @BINDIR@/omnewrotate.sh
toggle_in_files = omnewrotate.sh.in
toggle_DATA = $(toggle_in_files:.in=)

omnewrotate_SOURCES = \
	omnewrotate.sh \
	omnewrotate.c
