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
AC_CONFIG_AUX_DIR([tinfra-support/autoconf])
AC_CONFIG_SRCDIR([tinfra.bkl])
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

AC_CHECK_HEADERS([pthread.h time.h])

AC_CHECK_FUNCS([opendir nanosleep usleep])

case "$target" in
*-mingw*|*-mingw32*)
	LIBS="$LIBS -lwsock32"
	;;
*)
        LIBS="$LIBS -lpthread"
        ;;
esac
AC_CHECK_LIB(socket, socket)
AC_CHECK_LIB(nsl,gethostbyname)

AC_CHECK_FUNC(hstrerror)

AC_CONFIG_FILES([Makefile  tinfra.pc tinfra-test.pc])
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
