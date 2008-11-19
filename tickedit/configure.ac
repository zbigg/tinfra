#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([tickedit], [0.1], [Zbigniew Zagorski <z.zagorski@gmail.com>])
AC_CONFIG_AUX_DIR([build/autoconf-scripts])

AC_CANONICAL_HOST

AC_CONFIG_SRCDIR([tickedit.bkl])
AC_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_AWK
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB

AC_BAKEFILE([m4_include(autoconf_inc.m4)])


AM_OPTIONS_WXCONFIG

AM_PATH_WXCONFIG(2.8.0, WXFOUND=1)

if test "$WXFOUND" != 1; then
    AC_MSG_ERROR([
        Please check that wx-config is in path, the directory
        where wxWindows libraries are installed (returned by
        'wx-config --libs' command) is in LD_LIBRARY_PATH or
        equivalent variable and wxWindows is version 2.6.0 or above.
    ])
fi


AC_CONFIG_FILES([Makefile])
AC_OUTPUT

