#!/bin/sh
#
# this script renames instance of __cxa_throw to __cxa_throw_orig
# so an statically linked application
# can define own __cxa_throw and thus
# hijack exceptions :)
#
# z.zagorski@gmail.com
#
# implementation note,
# the easiest way to do is just to
#   --redefine-sym __cxa_throw=__cxa_throw_orig
# on whole library, but this renames also _users_, so one must
# find .o which contains implementation of wrapped function
# and rename symbol __only__ within this object file 

set -e -x

if [ -f "$1" ] ; then
    libfile="$1"
else
    libfile=`g++ -print-file-name=libstdc++.a`
fi

libfile_out="libstdc++-tinfra-throw.a"

ar x "$libfile" eh_throw.o
objcopy --redefine-sym __cxa_throw=__libstdcxx_cxa_throw eh_throw.o eh_throw2.o
mv eh_throw2.o eh_throw.o
cp $libfile $libfile_out
ar r $libfile_out eh_throw.o
ranlib $libfile_out

nm $libfile_out | grep cxa_throw

