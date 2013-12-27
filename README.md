tinfra - this is a next experimental C++ providing general programmer support for "everything(tm)".
===================

[![Build Status](https://travis-ci.org/zbigg/tinfra.png)](https://travis-ci.org/zbigg/tinfra)

Some highlights:

* os abstraction

  * filesystem abstraction (tinfra/fs.h)
  * subprocess (tinfra/subprocess.h)
  * threads (tinfra/thread.h, tinfra/mutex.h)

* infrastructure for type safe structure visitors (tinfr/mo.h)

  * yet another "to_string" infrastructure (tinfra/lex.h)
  * yet another "serialization" infrastructure

* many string helpers (ideas borrowed from perl&python)

  * type safe printf - tprintf, tsprintf (tinfra/fmt.h)

* json parser & generator (tinfra/json.h)
* runtime helpers for handling exceptions, interruptions, stack-traces etc
* simple unit-test library based on unittest++ concepts

Content
------------

There are two libraries:

* tinfra
    Base tinfra library for use in normal code.

    No dependencies.

    API Headers: all but tinfra/test*.h

    LIB: libtinfra.a or tinfra.lib 

* tinfra-test
    Simple unit-test helper code inspired by UnitTest++ (http://unittest-cpp.sourceforge.net/)

    Headers: tinfra/test.h, tinfra/test-macros.h

    LIB: libtinfra-test.a or tinfra-test.lib 

Tests of library are under unittests(.exe) target. They cover something like 50% of 
current code and 20-30% of functionality.

Installation, Linux/Unix or MSYS/Mingw, OSX
-------------------------------------------

Prerequisities:

* GNU make
* makefoo (https://github.com/zbigg/makefoo)
* C++ compiler (g++, clang++, XCode ...)

To build:

    ./configure --prefix=???
    make

(out-of-source build is supported)

To install:

    make install

(installs ${exec_prefix}/lib/libtinfra.a & libtinfra-test.a and ${prefix}/include/tinfra/...)

To check:

    make test

Installation MS VS 2012
--------------------------------

Prerequisities:

* makefoo (https://github.com/zbigg/makefoo)
* MSYS with GNU make is needed to run configure
* Microsoft Visual Studio 2012 Express

Build & configure:

    TOOLSET=msvs ./configure --prefix=???
    make

