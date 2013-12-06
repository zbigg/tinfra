dnl
dnl File:	configure.in
dnl Id:		$Id$
dnl Author:	Zbigniew Zagorski <z.zagorski@gmail.com>
dnl
dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(
    [tinfra-regexp],
    [0.0.1],
    [z.zagorski@gmail.com])

AC_CONFIG_AUX_DIR([build/autoconf])
AC_CONFIG_SRCDIR([tinfra/regexp.h])
AC_CONFIG_HEADER([tinfra/regexp-config.h])

AC_MAKEFOO

# Checks for programs.
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB

AC_CANONICAL_SYSTEM

PKG_CHECK_MODULES([DEPS], [tinfra >= 0.0.2 tinfra-test libpcre >= 7.8],
	[
	LIBS="$DEPS_LIBS $LIBS"
	CXXFLAGS="$DEPS_CFLAGS $CXXFLAGS"
	CPPFLAGS="$DEPS_CFLAGS $CFLAGS"
	])

AC_DEFINE(HAVE_PCRE,1,[Have PCRE library for regexp])

AC_CONFIG_FILES([Makefile tinfra-regexp.pc])
AC_OUTPUT

cat << EOF
Configured tinfra-regexp with options:

    CFLAGS                  $CFLAGS
    CXXFLAGS                $CXXFLAGS
    CPPFLAGS                $CPPFLAGS
    LDFLAGS                 $LDFLAGS
    LIBS                    $LIBS
    Installation location   $prefix

Type
    \$make
to build, then type
    \$sudo make install
to install stuff.
EOF

