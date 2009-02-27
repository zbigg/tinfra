//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef __tinfra__platform_h__
#define __tinfra__platform_h__

#ifdef _MSC_VER
#pragma warning( disable: 4512) // assignment operator could not be generated
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

#endif
