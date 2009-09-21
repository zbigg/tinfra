dnl
dnl File:	configure.in
dnl Id:		$Id$
dnl Author:	Zbigniew Zagorski <z.zagorski@gmail.com>
dnl
dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(
    [tinfra-xml],
    [0.0.1],
    [z.zagorski@gmail.com])

AC_CONFIG_AUX_DIR([tinfra-support/autoconf])
AC_CONFIG_SRCDIR([tinfra-xml.bkl])
AC_CONFIG_HEADER([tinfra/xml-config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB

AC_CANONICAL_SYSTEM

AC_BAKEFILE([m4_include(autoconf_inc.m4)])

PKG_CHECK_MODULES([TINFRA], [tinfra >= 0.0.1],
    [
        CXXFLAGS="$CXXFLAGS $TINFRA_CFLAGS"
        CPPFLAGS="$CPPFLAGS $TINFRA_CFLAGS"
        LIBS="$LIBS $TINFRA_LIBS"
    ],
    [ AC_MSG_ERROR([
            tinfra library required
        ]) ])

AC_CHECK_LIB(expat, XML_ParseBuffer, [], [
        AC_MSG_ERROR([ expat library required ])
        ])

AC_CONFIG_FILES([Makefile tinfra-xml.pc])
AC_OUTPUT

cat << EOF
Configured tinfra-xml with options:

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
