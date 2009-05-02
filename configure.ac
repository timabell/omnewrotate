AC_PREREQ(2.53)
AC_INIT(omnewrotate, 0.5.4, http://code.google.com/p/omnewrotate/)
AM_INIT_AUTOMAKE()
AC_CONFIG_SRCDIR(src/)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

AC_GNU_SOURCE
AC_ISC_POSIX
AC_PROG_CC
AC_STDC_HEADERS
AC_PROG_LIBTOOL

PKG_CHECK_MODULES(XRANDR, xrandr, [],
		[AC_MSG_FAILURE([xrandr not found])]
		)

AC_ARG_WITH([frameworkd], AS_HELP_STRING([--without-frameworkd], [Build without frameworkd-glib0 library (default: test)]))

if test "x$with_frameworkd" != "xno"; then
	PKG_CHECK_MODULES(FRAMEWORKD_GLIB, libframeworkd-glib, [],
		[AC_MSG_FAILURE([libframeworkd-glib not found])]
		)

	AC_DEFINE([HAVE_LIBFRAMEWORKD_GLIB], [1], [Whether to use libframeworkd_glib])
fi

PTHREAD_CFLAGS="-pthread"
PTHREAD_LIBS="-lpthread"
AC_SUBST(PTHREAD_CFLAGS)
AC_SUBST(PTHREAD_LIBS)

BINDIR=`eval echo $bindir`
AC_SUBST([BINDIR])
DATADIR=`eval echo $datadir`
AC_SUBST([DATADIR])

XSESSIONDIR="${sysconfdir}/X11/Xsession.d"
AC_SUBST([XSESSIONDIR])

CONFIGDIR="${sysconfdir}/default"
AC_SUBST([CONFIGDIR])

APMDIR="${sysconfdir}/apm"
AC_SUBST([APMDIR])

AC_SUBST([VERSION])

dnl ==============================================
dnl DBUS
dnl ==============================================

DBUS_GLIB_BIN="`$PKG_CONFIG --variable=exec_prefix dbus-glib-1`/bin"

dnl AC_ARG_WITH(dbusbindir,
dnl            AC_HELP_STRING([--with-dbusbindir=[=DIR]],
dnl                           [Use a different path to dbus tools]),,
dnl                           dbusdir=$withval;)

AC_ARG_WITH(dbusbindir,[  --with-dbusbindir=DIR  dbus tools in DIR], [DBUS_GLIB_BIN="$withval"])
echo "DBus tools location ${DBUS_GLIB_BIN}"
AC_SUBST(DBUS_GLIB_BIN)

AC_OUTPUT([
Makefile
data/Makefile
data/omnewrotate.desktop
data/11omnewrotate-suspend
data/89omnewrotate-resume
data/89omnewrotate
data/omnewrotate.sh
data/omnewrotate.conf
src/Makefile
])
