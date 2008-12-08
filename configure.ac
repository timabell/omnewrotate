AC_PREREQ(2.53)
AC_INIT(omnewrotate, 0.5.0, http://code.google.com/p/omnewrotate/)
AM_INIT_AUTOMAKE()
AC_CONFIG_SRCDIR(src/)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

AC_GNU_SOURCE
AC_ISC_POSIX
AC_PROG_CC
AC_STDC_HEADERS
AC_PROG_LIBTOOL

PKG_CHECK_MODULES(DBUS, xrandr)
PKG_CHECK_MODULES(FRAMEWORKD_GLIB,
          libframeworkd-glib,
		  [],
		  [AC_MSG_FAILURE([libframeworkd-glib not found])]
		 )
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
src/Makefile
])
