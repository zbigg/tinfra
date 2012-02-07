//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#ifndef tinfra_platform_h_included
#define tinfra_platform_h_included

#if defined _MSC_VER
//
// excerpt from MSDN:
//
//  __FUNCDNAME__
//      ... returns the decorated name of the enclosing function (as a string)...
// __FUNCSIG__
//      ... returns the signature of the enclosing function (as a string). ...
// __FUNCTION__
//      ... returns the undecorated name of the enclosing function (as a string). ...
//
#define TINFRA_PRETTY_FUNCTION __FUNCSIG__
#define TINFRA_SHORT_FUNCTION __FUNCTION__
#elif defined(__GNUC__)
#define TINFRA_PRETTY_FUNCTION __PRETTY_FUNCTION__
#define TINFRA_SHORT_FUNCTION __func__
#else
#define TINFRA_PRETTY_FUNCTION __func__
#define TINFRA_SHORT_FUNCTION __func__
#endif
    


#ifdef _MSC_VER
#pragma warning( disable: 4512) // assignment operator could not be generated
#pragma warning( disable: 4511) // copy constructor could not be generated
#pragma warning( disable: 4312) // 
#pragma warning( disable: 4127) // conditional expression is constant

// some strange warning disablers for recent visuals
// see http://msdn.microsoft.com/en-us/library/aa985965(VS.80).aspx

#define _CRT_SECURE_NO_WARNINGS  1 // yeah, good idea. warn about correct C++ usages
#define _CRT_NONSTDC_NO_WARNINGS 1 // because of someone paranoia
#define _SCL_SECURE_NO_WARNINGS  1 // this one is extremly stupid 

#undef HAVE_SYS_TIME_H
#undef HAVE_OPENDIR

#define HAVE_TIME_H 
#define HAVE_IO_H
#define HAVE_FINDFIRST


#else
#include "tinfra/config.h"
#endif

#include <ios>
#include <cstddef>

namespace tinfra {
	using std::size_t;
	using std::streamsize;
	using ::intptr_t;
}

#ifdef __GNUC__

#define TINFRA_LIKELY(x)       __builtin_expect((x),true)
#define TINFRA_UNLIKELY(x)     __builtin_expect((x),false)

#else

#define TINFRA_LIKELY(x)       (x)
#define TINFRA_UNLIKELY(x)     (x)

#endif

#if defined(__GXX_EXPERIMENTAL_CXX0X__) || (__cplusplus >= 201103L)
#define TINFRA_CXX11
#define TINFRA_HAS_VARIADIC_TEMPLATES
#endif

#endif // tinfra_platform_h_included
