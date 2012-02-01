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

ar x libstdc++.a eh_throw.o
objcopy --redefine-sym __cxa_throw=__cxa_throw_orig eh_throw.o eh_throw2.o
mv eh_throw2.o eh_throw.o
cp libstdc++.a libstdc++-my.a
ar r libstdc++-my.a eh_throw.o
 
