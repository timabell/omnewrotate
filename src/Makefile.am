INTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = -DPKGDATADIR=\"$(pkgdatadir)\" \
                $(FRAMEWORKD_GLIB_CFLAGS) \
                -Wall

AM_LDFLAGS = @FRAMEWORKD_GLIB_LIBS@

bin_PROGRAMS=omnewrotate

omnewrotate_SOURCES = \
	omnewrotate.c
