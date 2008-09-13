dnl
dnl File:	configure.in
dnl Id:		$Id$
dnl Author:	Zbigniew Zagorski <z.zagorski@gmail.com>
dnl
dnl
dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(
    tinfra,
    0.0.1 ,
    [z.zagorski@gmail.com])
AC_CONFIG_AUX_DIR([build/autoconf])
AC_CONFIG_SRCDIR([tinfra/tinfra.h])
AC_CONFIG_HEADER([tinfra/config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB

AC_CANONICAL_SYSTEM


AC_BAKEFILE([m4_include(autoconf_inc.m4)])

m4_include([build/autoconf/pcre.m4])
AC_CHECK_HEADERS([pthread.h])

AC_CHECK_FUNCS([opendir nanosleep usleep])

case "$target" in
*-mingw*|*-mingw32*)
	LIBS="$LIBS -lwsock32"
	;;
esac
AC_CHECK_LIB(socket, socket)
AC_CHECK_LIB(nsl,gethostbyname)

AC_CHECK_FUNC(hstrerror)

AM_PATH_PCRE(7.0, 
[
    LIBS="$LIBS $PCRE_LIBS"
    CPPFLAGS="$CPPFLAGS $PCRE_CFLAGS"
    AC_DEFINE(HAVE_PCRE,1,[Have PCRE library for regexp])
], 
[
    AC_MSG_ERROR([pcre library not found, where is your pcre-config ?])
])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

cat << EOF
Configured tinfra with options:

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
