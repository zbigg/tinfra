dnl
dnl File:	configure.in
dnl Id:		$Id$
dnl Author:	Zbigniew Zagorski <z.zagorski@gmail.com>
dnl
dnl
dnl Process this file with autoconf to produce a configure script.

AC_PREREQ([2.59])
AC_INIT([tinfra],[0.0.2],[z.zagorski@gmail.com])

m4_include([build/m4/makefoo.m4])

AC_CONFIG_AUX_DIR([build/autoconf])
AC_CONFIG_SRCDIR([tinfra/mo.h])
AC_CONFIG_HEADER([tinfra/config-priv-autoconf.h])
AC_CONFIG_HEADER([tinfra/config-pub-autoconf.h])

AC_MAKEFOO

#CXXFLAGS="$CXXFLAGS -std=c++0x"
# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_RANLIB

AC_CANONICAL_TARGET

case "$target" in
*-mingw*|*-mingw32*)
	LIBS="$LIBS -lws2_32"
        CXXFLAGS="$CXXFLAGS -mthreads"
        CLIENT_CXXFLAGS="$CLIENT_CXXFLAGS -mthreads"
        LDFLAGS="$LDFLAGS -mthreads -Wl,--enable-auto-import"
	;;
*)
        LIBS="$LIBS -lpthread"
        ;;
esac

AC_SUBST(CLIENT_CXXFLAGS)


AC_CHECK_HEADERS([pthread.h],
    AC_DEFINE(TINFRA_HAVE_PTHREAD_H,1,[Have pthread.h with posix threads])
    )
AC_CHECK_HEADERS([time.h execinfo.h])
AC_CHECK_FUNCS([opendir nanosleep usleep backtrace hstrerror strnicmp strncasecmp])

AC_SEARCH_LIBS([socket], [socket], 
    [AC_DEFINE([HAVE_SOCKET], [1], [socket function available])])
    
AC_SEARCH_LIBS([gethostbyname],[nsl],
    [AC_DEFINE([HAVE_GETHOSTBYNAME], [1], [gethostbyname function available])])
    
AC_SEARCH_LIBS([clock_gettime],[rt],
    [AC_DEFINE([HAVE_CLOCK_GETTIME], [1], [clock_gettime function available])])

AC_SEARCH_LIBS([dladdr], [dl], 
    [AC_DEFINE([HAVE_DLADDR], [1], [dladdr function available])])

TINFRA_XML_USE_EXPAT_PARSER=0
AC_CHECK_HEADERS([expat.h],[
    AC_DEFINE(TINFRA_EXPAT_H,1,[Have expat.h with XML SAX parser])
    AC_CHECK_LIB(expat, XML_ParseBuffer,
        [
            TINFRA_XML_USE_EXPAT_PARSER=1
            TINFRA_XML_LIBS=-lexpat
        ])
    ])
AC_SUBST(TINFRA_XML_USE_EXPAT_PARSER)
AC_SUBST(TINFRA_XML_LIBS)

AC_CHECK_HEADERS([regex.h],
    AC_DEFINE(TINFRA_HAVE_REGCOMP,1,[Have regex.h with posix threads])
    )

TINFRA_HAVE_PCRE=0
PKG_CHECK_MODULES([PCRE], [libpcre >= 7.8],
	[
        AC_DEFINE(TINFRA_PCRE,1,[Have PCRE - perl compatible regural expression library])
        TINFRA_REGEXP_LIBS="$PCRE_LIBS"
        TINFRA_REGEXP_CXXFLAGS="$PCRE_CFLAGS"
        TINFRA_HAVE_PCRE=1
	])

AC_SUBST(TINFRA_REGEXP_LIBS)
AC_SUBST(TINFRA_REGEXP_CXXFLAGS)
AC_SUBST(TINFRA_HAVE_PCRE)

AC_CONFIG_FILES([Makefile tinfra.pc tinfra-test.pc tinfra-xml.pc])
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
