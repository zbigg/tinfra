//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#ifndef tinfra_platform_h_included
#define tinfra_platform_h_included

#ifdef _WIN32
#define TINFRA_W32
#else
#define TINFRA_POSIX
#endif

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

//
// C++ 11
//
#if defined(__GXX_EXPERIMENTAL_CXX0X__) || (__cplusplus >= 201103L)
#define TINFRA_CXX11
#define TINFRA_HAS_VARIADIC_TEMPLATES
#endif

//
// standard sizes in tinfra
//
#include <cstddef>
#ifdef TINFRA_CXX11
#include <cstdint>  // for intptr_t as defined in C++11
#else
#include <stdint.h> // for intptr_t as deined in C99 and available in most places
#endif

namespace tinfra {
	using std::size_t;
	using ::intptr_t;
}

//
// LIKELY & UNLIKELY
//
#ifdef __GNUC__

#define TINFRA_LIKELY(x)       __builtin_expect((x),true)
#define TINFRA_UNLIKELY(x)     __builtin_expect((x),false)

#else

#define TINFRA_LIKELY(x)       (x)
#define TINFRA_UNLIKELY(x)     (x)

#endif

#endif // tinfra_platform_h_included

