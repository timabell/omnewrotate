INTAINERCLEANFILES = Makefile.in

INCLUDES = -I /usr/local/openmoko/arm/arm-angstrom-linux-gnueabi/usr/include

AM_CPPFLAGS = -DPKGDATADIR=\"$(pkgdatadir)\" \
                $(FRAMEWORKD_GLIB_CFLAGS) \
                -Wall

AM_LDFLAGS = @FRAMEWORKD_GLIB_LIBS@ -L /usr/local/openmoko/arm/arm-angstrom-linux-gnueabi/usr/lib

bin_PROGRAMS=omnewrotate

omnewrotate_SOURCES = \
	omnewrotate.c
